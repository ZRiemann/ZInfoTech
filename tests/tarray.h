#ifndef _ZTST_ARRAY_H_
#define _ZTST_ARRAY_H_
#include "tcommon.h"
#include <zit/container/array.h>
#include <zit/base/time.h>

#define ZARR_CHECK 1

#ifdef __cplusplus
/*
 * zit.array VS std::set performance
 */
#include <set>

static void tset(int size){
    std::set<int> st;
    ztick_t tick = NULL;
    int idx = 0;

    /*
     * test insert performance
     */
    ZDBG("test std::set insert<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        st.insert(idx);
    }
    ztock(tick, NULL, NULL);
    /*
     * test find performance
     */
    ZDBG("test std::set find<%d> performance", size);
    std::set<int>::iterator it;
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
#if ZARR_CHECK
        it = st.find(idx);
        if(it == st.end()){
            ZERR("Not find %d", idx);
        }
#else
        it = st.find(idx);
#endif
    }
    ztock(tick, NULL, NULL);
    /*
     * test erase performance
     */
    ZDBG("test std::set erase<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        st.erase(st.find(idx));
    }
    ztock(tick, NULL, NULL);
}
#endif /* __cplusplus */

static void tdump_array(zarray_t *arr){
    int i = 0;
    zarray_foreach(arr, print_iany, &i);
    printf("\n");
}
static void tarray_dichotomy_search(zarray_t *arr, int size){
    ztick_t tick = NULL;
    zany_t value = {0};
    int idx = 0;
    int pos = 0;
#if ZARR_CHECK
    zerr_t ret = ZEOK;
#endif
    /*
     * test insert performance
     */
    ZDBG("test zit.array insert<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        value.i = idx;
        zarray_insert(arr, value);
    }
    ztock(tick, NULL, NULL);
    //tdump_array(arr);
    /*
     * test find performance
     */
    ZDBG("test zit.array find<%d> performance", size);
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        value.i = idx;
#if ZARR_CHECK
        ret = zarray_find(arr, value, &pos); /* Check function logic */
        if(ret != ZEQUAL){
            ZERR("Not find %d", idx);
        }
#else
        zarray_find(arr, value, &pos); /* Calculate average us/call */
#endif
    }
    ztock(tick, NULL, NULL);
    /*
     * test erase performance
     */
    ZDBG("test zit.array erase<%d> performance", size);
#if 1
    value.i = 0;
    tick = ztick();
    zarray_erase(arr, value); /* Calculate max us/call */
    ztock(tick, NULL, NULL);
#else
    tick = ztick();
    for(idx = 0; idx < size; ++idx){
        value.i = idx;
#if ZARR_CHECK
        ret = zarray_erase(arr, value); /* Check function logic */
        if(ret != ZEQUAL){
            ZERR("Not find %d", idx);
        }
#else
        zarray_erase(arr, value); /* Calculate average us/call */
#endif
    }
    ztock(tick, NULL, NULL);
#endif
}

static void tarray(int argc, char **argv){
    zerr_t ret = ZEOK;
    zarray_t *arr = 0;
    zany_t val = {0};
    int i = 0;
    int cnt = 64;
    if(argc == 3){
        cnt = atoi(argv[2]);
    }
    ret = zarray_create(&arr, 1024);ZERRC(ret);
    for(i=0; i<cnt; i++){
        val.i = i;
        ret = zarray_insert(arr, val);ZERRC(ret);
    }
    tdump_array(arr);
    for(i=0; i<cnt; i++){
        val.i = i;
        ret = zarray_erase(arr, val);
        tdump_array(arr);
    }
    /*
     * desc insert
     */
    i = cnt;
    while(i>=0){
        val.i = i;
        ret = zarray_insert(arr, val);ZERRC(ret);
        --i;
    }
    tdump_array(arr);
    i = cnt;
    while(i>=0){
        val.i = i;
        ret = zarray_erase(arr, val);
        tdump_array(arr);
        --i;
    }

    /*
     * Test dichotomy search performance 5,000 item performance
     */
    tarray_dichotomy_search(arr, 5000);
    /*
     * Test dichotomy search performance 50,000 item performance
     */
    tarray_dichotomy_search(arr, 50000);
    /*
     * Test dichotomy search performance 500,000 item performance
     */
    tarray_dichotomy_search(arr, 500000);
    /*
     * Test dichotomy search performance 5,000,000 item performance
     */
    tarray_dichotomy_search(arr, 5000000);
    ret = zarray_destroy(arr); ZERRC(ret);
#ifdef __cplusplus
    tset(5000);
    tset(50000);
    tset(500000);
    tset(5000000);
#endif
}

#endif
