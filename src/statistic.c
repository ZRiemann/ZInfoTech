/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#include "export.h"
#include <zit/statistic/statistic.h>
#include <zit/base/error.h>
#include <stdio.h>

/*
 * inline implements
 */
static zerr_t zstatistic_init_hots(zststc_hots_t *hots){
    /*
     * Init at start, memory error impossible at normal system.
     * if error, maybe system in error status first.
     */
    hots->hits = zatm_alloc_atm64();
    zatm64_xchg(hots->hits, 0);
    hots->total_time = zatm_alloc_atm64();
    zatm64_xchg(hots->total_time, 0);
    hots->hots_idx = zatm_alloc_atm();
    zatm_xchg(hots->hots_idx, 0);
    return ZEOK;
}

static zerr_t zstatistic_fini_hots(zststc_hots_t *hots){
    zatm_free(hots->hits);
    zatm_free(hots->total_time);
    zatm_free(hots->hots_idx);
    return ZEOK;
}

zerr_t zststc_init(zststc_t *stc, int hots_size, int *hots_fns){
    int i, j;

    /* Initialize hotspots */
    stc->hots = (zststc_hots_t**)calloc(hots_size, sizeof(zststc_hots_t*));
    for(i=0 ; i<hots_size ; ++i){
        if(hots_fns[i]){
            stc->hots[i] = calloc(hots_fns[i], sizeof(zststc_hots_t));
            for(j=0; j<hots_fns[i]; ++j){
                zstatistic_init_hots(&stc->hots[i][j]);
            }
        }else{
            stc->hots[i] = NULL;
        }
    }
    stc->hots_size = hots_size;
    stc->hots_fns = hots_fns;

    /* Initialize thread call stack */
    zspin_init(&stc->spin);
    stc->sfn = (char***)calloc(hots_size, sizeof(char**));
    for(i=0; i<hots_size; ++i){
        if(stc->hots_fns[i]){
            stc->sfn[i] = (char**)calloc(hots_fns[i], sizeof(char*));
        }else{
            stc->sfn[i] = NULL;
        }
    }
    zmap_create(&stc->thr_stack, 64);
    return ZOK;
}

zerr_t zststc_fini(zststc_t *stc){
    uint32_t i, j;
    for(i=0; i<(uint32_t)stc->hots_size; ++i){
        if(stc->hots[i]){
            for(j=0; j<(uint32_t)stc->hots_fns[i]; ++j){
                zstatistic_fini_hots(&stc->hots[i][j]);
            }
            free(stc->hots[i]);
        }
    }
    free(stc->hots);

    /* Finish thread call stack */
    for(i=0; i<(uint32_t)stc->hots_size; ++i){
        free(stc->sfn[i]);
    }
    free(stc->sfn);

    for(j = 0; j< stc->thr_stack->size; ++j){
        zque1_destroy(stc->thr_stack->pair[j].value.p);
    }
    zmap_destroy(stc->thr_stack);
    zspin_fini(&stc->spin);
    return ZOK;
}

void zststc_add_thrs(zststc_t *stc, const char *thr_name){
    ztid_t tid = zthread_self();
    zcontainer_t que = NULL;
    int capacity = 32;
    zpair_t pair = {0};

    zque1_create(&que, (zvalue_t)&capacity);
    zque1_push(que, (zvalue_t)thr_name);
    pair.key.u32 = tid;
    pair.value.p = que;
    zspin_lock(&stc->spin);
    zmap_insert_pair(stc->thr_stack, &pair);
    zspin_unlock(&stc->spin);
}


static void zstatistic_dump_hots_org(FILE *pf, zststc_hots_t *hots){
    int idx;
    char ts_str[64];
    ztime_stamp_t *ts;
    fprintf(pf, "  - hits:\t%lu\n", *hots->hits);
    fprintf(pf, "  - total_time:\t%lu(us)\n", *hots->total_time);
    idx = *hots->hots_idx & 0xff;
    fprintf(pf, "  - real time:\n");
    for(int i=0; i<256; ++i){
        fprintf(pf, "%s%llu", i%16 ? " " : "\n    ", (long long unsigned int)hots->real_time[(i+idx+1)&0xff]);
    }
    fprintf(pf, "\n  - hotspot:\n");
    for(int i=0; i<256; ++i){
        ts = &hots->hotspot[(i+idx+1)&0xff];
        ztime_ts2str(ts, ts_str);
        fprintf(pf, "%s%s", i%8 ? " " : "\n    ", ts_str);
    }
}

static zerr_t zdump_thr_stack(ZOP_ARG){
    fprintf((FILE*)hint, "   %s\n", (char*)in);
    return ZOK;
}
void zststc_dump_org(zststc_t *stc, const char *path){
    FILE *pf;
    char fname[256];
    ztime_t date;
    uint32_t i, j;
    zget_localtime(&date);
    sprintf(fname, "%sstatistic%02d%02d%02d%02d.org", path ? path : "", date.day, date.hour, date.minute, date.second);
    pf = fopen(fname, "w");
    if(!pf){
        return;
    }
    zatm_barrier();
    /* Dump threads call stack */
    fprintf(pf, "* Thread call stack\n");
    for(i=0; i<stc->thr_stack->size; i++){
        fprintf(pf, "** Thread[%u]\n", stc->thr_stack->pair[i].key.u32);
        zque1_foreach(stc->thr_stack->pair[i].value.p, zdump_thr_stack, pf);
    }
    /* Dump hotspot  */
    for(i=0; i<(uint32_t)stc->hots_size; ++i){
        fprintf(pf, "* SRI[%d]\n", i);
        if(stc->hots[i]){
            for(j=0; j<(uint32_t)stc->hots_fns[i]; ++j){
                fprintf(pf, "** SRI[%d]SFI[%d]: %s\n", i, j, stc->sfn ? stc->sfn[i][j] : "");
                zstatistic_dump_hots_org(pf, &(stc->hots[i][j]));
                fprintf(pf, "\n");
            }
        }
    }

    fclose(pf);
}
