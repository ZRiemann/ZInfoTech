#ifndef _ZTST_MAP_H_
#define _ZTST_MAP_H_
#include "tcommon.h"
#include <zit/container/map.h>
#include <zit/base/time.h>

#define ZMAP_CHECK 0

#ifdef __cplusplus
/*
 * std::map VS zit.map
 */
#include <map>

static void tstd_map(int size){
    std::map<int, int> mp;
    ztick_t tick = NULL;
    int idx = 0;

    /*
     * test insert performance
     */
    ZDBG("test std::map insert<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        mp.insert(std::pair<int, int>(idx, idx));
    }
    ztock(tick, NULL, NULL);
    /*
     * test find performance
     */
    ZDBG("test std::map find<%d> performance", size);
    std::map<int, int>::iterator it;
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
#if ZMAP_CHECK
        it = mp.find(idx);
        if(it == mp.end()){
            ZERR("Not find %d", idx);
        }
#else
        it = mp.find(idx);
#endif
    }
    ztock(tick, NULL, NULL);
    /*
     * test erase performance
     */
    ZDBG("test std::map erase<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        mp.erase(mp.find(idx));
    }
    ztock(tick, NULL, NULL);
}

#endif /* __cplusplus */

static void tdump_map(zmap_t *mp){
    int i = 0;
    zmap_foreach(mp, print_iany, &i);
    printf("\n");
}
static void tmap_dichotomy_search(zmap_t *mp, int size){
    ztick_t tick = NULL;
    zany_t key = {0};
    zany_t value = {0};
    zpair_t *pair = NULL;
    int idx = 0;
    int pos = 0;
#if ZMP_CHECK
    zerr_t ret = ZEOK;
#endif
    /*
     * test insert performance
     */
    ZDBG("test zit.map insert<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        key.i = idx;
        value.i = idx;
        zmap_insert(mp, key, value);
    }
    ztock(tick, NULL, NULL);
    /*
     * test find performance
     */
    ZDBG("test zit.map find<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        value.i = idx;
#if ZMP_CHECK
        ret = zmap_find(mp, key, &pair, &pos); /* Check function logic */
        if(ret != ZEQUAL){
            ZERR("Not find %d", idx);
        }
#else
        zmap_find(mp, key, &pair, &pos); /* Calculate average us/call */
#endif
    }
    ztock(tick, NULL, NULL);
    /*
     * test erase performance
     */
    ZDBG("test zit.map erase<%d> performance", size);
#if 1
    key.i = 0;
    tick = ztick();
    zmap_erase(mp, key, &value); /* Calculate max us/call */
    ztock(tick, NULL, NULL);
#else
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        key.i = idx;
#if ZMP_CHECK
        ret = zmap_erase(mp, key, &value); /* Check function logic */
        if(ret != ZEQUAL){
            ZERR("Not find %d", idx);
        }
#else
        zmap_erase(mp, key, &value); /* Calculate average us/call */
#endif
    }
    ztock(tick, NULL, NULL);
#endif
}

static void tmap(int argc, char **argv){
    zerr_t ret = ZEOK;
    zmap_t *mp = 0;
    zany_t key = {0};
    zany_t val = {0};

    int i = 0;
    int cnt = 64;
    if(argc == 3){
        cnt = atoi(argv[2]);
    }
    ret = zmap_create(&mp, 1024);ZERRC(ret);
    for(i=0; i<cnt; i++){
        val.i = i;
        key.i = i;
        ret = zmap_insert(mp, key, val);ZERRC(ret);
    }
    tdump_map(mp);
    for(i=0; i<cnt; i++){
        key.i = i;
        ret = zmap_erase(mp, key, &val);
        tdump_map(mp);
    }
    /*
     * desc insert
     */
    i = cnt;
    while(i>=0){
        val.i = i;
        key.i = i;
        ret = zmap_insert(mp, key, val);ZERRC(ret);
        --i;
    }

    tdump_map(mp);
    i = cnt;
    while(i>=0){
        key.i = i;
        ret = zmap_erase(mp, key, &val);
        tdump_map(mp);
        --i;
    }

    /*
     * Test dichotomy search performance 5,000 item performance
     */
    tmap_dichotomy_search(mp, 5000);
    /*
     * Test dichotomy search performance 50,000 item performance
     */
    tmap_dichotomy_search(mp, 50000);
    /*
     * Test dichotomy search performance 500,000 item performance
     */
    tmap_dichotomy_search(mp, 500000);
    /*
     * Test dichotomy search performance 5,000,000 item performance
     */
    //tmap_dichotomy_search(mp, 5000000);
    ret = zmap_destroy(mp); ZERRC(ret);
#ifdef __cplusplus
    tstd_map(5000);
    tstd_map(50000);
    tstd_map(500000);
    tstd_map(5000000);
#endif
}

#endif
