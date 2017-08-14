#include "export.h"
#include <zit/statistic/statistic.h>
#include <zit/base/error.h>
#include <stdio.h>

/*
 * inline implements
 */
static zerr_t zstatistic_init_hots(zstat_hots_t *hots){
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
    hots->stack_idx = zatm_alloc_atm();
    zatm_xchg(hots->stack_idx, 0);
    return ZEOK;
}

static zerr_t zstatistic_fini_hots(zstat_hots_t *hots){
    zatm_free(hots->hits);
    zatm_free(hots->total_time);
    zatm_free(hots->hots_idx);
    zatm_free(hots->stack_idx);
    return ZEOK;
}

zerr_t zstatistic_init(zstat_hots_t ***hots, int n, int *size){
    int i, j;
    *hots = (zstat_hots_t**)calloc(n, sizeof(zstat_hots_t*));
    for(i=0 ; i<n ; ++i){
        if(size[i]){
            (*hots)[i] = calloc(size[i], sizeof(zstat_hots_t));
            for(j=0; j<size[i]; ++j){
                zstatistic_init_hots(&((*hots)[i][j]));
            }
        }else{
            (*hots)[i] = NULL;
        }
    }
    return ZEOK;
}

zerr_t zstatistic_fini(zstat_hots_t ***hots, int n, int *size){
    int i, j;
    for(i=0; i<n; ++i){
        if((*hots)[i]){
            for(j=0; j<size[i]; ++j){
                zstatistic_fini_hots(&((*hots)[i][j]));
            }
            free((*hots)[i]);
        }
    }
    free(*hots);
    return ZEOK;
}

void  zstatistic_push_hots(zstat_hots_t **hots, zstat_node_t *node, int sri, int sfi){
    node->hots = &(hots[sri][sfi]);
    node->tid = zthread_self();
    node->idx = zatm_inc(node->hots->stack_idx) & 0xff;
    node->hots->tid_stack[node->idx] = (node->tid | 0x10000000);
    ztime_stamp(&node->begin);
}

void zstatistic_pop_hots(zstat_node_t *node){
    zatm64_inc(node->hots->hits);
    node->idx = zatm_inc(node->hots->stack_idx) &0xff;
    node->hots->tid_stack[node->idx] = node->tid;
    ztime_stamp(&node->end);
    ztime_interval(&node->begin,  &node->end, &node->interval);
    node->idx = zatm_inc(node->hots->hots_idx) & 0xff;
    node->hots->hotspot[node->idx] = node->begin;
    node->hots->real_time[node->idx] = node->interval;
    zatm64_add(node->hots->total_time, node->interval);
}

static void zstatistic_dump_hots_file(FILE *pf, zstat_hots_t *hots){
    int idx;
    char ts_str[64];
    ztime_stamp_t *ts;
    fprintf(pf, "\thits:\t%lu\n", *hots->hits);
    fprintf(pf, "\ttotal_time:\t%lu(us)\n", *hots->total_time);
    idx = *hots->stack_idx & 0xff;
    fprintf(pf, "\tthreads stack:");
    for(int i=0; i<256; ++i){
        fprintf(pf, "%c%x", i%16 ? ' ' : '\n', hots->tid_stack[(i+idx+1)&0xff]);
    }
    idx = *hots->hots_idx & 0xff;
    fprintf(pf, "\nreal time:\n");
    for(int i=0; i<256; ++i){
        fprintf(pf, "%c%llx", i%16 ? ' ' : '\n', (long long unsigned int)hots->real_time[(i+idx+1)&0xff]);
    }
    fprintf(pf, "\nhotspot:\n");
    for(int i=0; i<256; ++i){
        ts = &hots->hotspot[(i+idx+1)&0xff];
        ztime_ts2str(ts, ts_str);
        fprintf(pf, "%c%s", i%8 ? ' ' : '\n', ts_str);
    }
}

void zstatistic_dump_file(zstat_hots_t **hots, int n, int *size, const char *path){
    FILE *pf;
    char fname[256];
    ztime_t date;
    int i, j;
    zget_localtime(&date);
    sprintf(fname, "%sstatistic%02d%02d%02d%02d.txt", path ? path : "", date.day, date.hour, date.minute, date.second);
    pf = fopen(fname, "w");
    if(!pf){
        return;
    }
    zatm_barrier();
    for(i=0; i<n; ++i){
        fprintf(pf, "SRI[%d]\n\n", i);
        if(hots[i]){
            for(j=0; j<size[i]; ++j){
                fprintf(pf, "\tSRI[%d] SFI[%d]\n", i, j);
                zstatistic_dump_hots_file(pf, &(hots[i][j]));
                fprintf(pf, "\n\n");
            }
        }
    }

    fclose(pf);
}
