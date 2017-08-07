#include <zit/container/list.h>

static void list_1r1w_push(zcontainer_t list, int begin, int end){
    zvalue_t val;
    for(int i = begin; i<end; ++i){
        ZCONVERT(val, i);
        zlist_push(list, val);
    }
}

static void list_1r1w_push_nocheck(zcontainer_t list, int begin, int end){
    //zvalue_t val;
    for(int i = begin; i<end; ++i){
        //ZCONVERT(val, i);
        zlist_push(list, 0);
        /*if( 0 == (i & 0xfffff)){
             ZDBG("push<%d>", i);
        }
        */
    }
}


static zerr_t tlist_compare(ZOP_ARG){
    int sub = (int)(hint-in);
    int *cmp = (int*)out;
    sub = sub ? (sub > 0 ? ZGREAT : ZLITTLE) : ZEQUAL;
    *cmp = sub;
    ZDBG("hint: %d in: %d cmp: %d", hint, in, sub);
    return ZOK;
}

static void list_1r1w_base(zcontainer_t list){
    zvalue_t val;
    int i;

    ZDBG("dump each value:");
    zlist_foreach(list, dump_int, NULL);
    list_1r1w_push(list, 0, 256);
    zlist_pop(list, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_frontk<%d>\n", i);
    zlist_popback(list, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_back<%d>\n", i);
    ZDBG("\ntesting zlist_foreach(1~254)\n");
    zlist_foreach(list, dump_int, NULL);
    ZDBG("\nbegin pop_fornt:\n");
    while(ZEOK == zlist_pop(list, &val)){
        dump_int(val, NULL, NULL);
    }
    list_1r1w_push(list, 0, 10);
    i = 3;
    ZCONVERT(val, i);
    zlist_insert(list, val, tlist_compare, ZLITTLE);
    ZDBG("\ndump insert 3:\n");
    zlist_foreach(list, dump_int, NULL);
    i = 4;
    ZCONVERT(val, i);
    zlist_erase(list, val, tlist_compare, ZEQUAL);
    ZDBG("\ndump earse EQUAL 4:\n");
    zlist_foreach(list, dump_int, NULL);
    i = 5;
    ZCONVERT(val, i);
    zlist_erase(list, val, tlist_compare, ZGREAT);
    ZDBG("\ndump earse ZGREAT 4:\n");
    zlist_foreach(list, dump_int, NULL);

    ZDBG("\nbegin pop_back:\n");
    while(ZEOK == zlist_popback(list, &val)){
        dump_int(val, NULL, NULL);
    }
}

zthr_ret_t ZCALL zproc_list_push_nocheck(void* list){
    ztick_t tick;
    int sec,usec;
    int i = 1024*1024;
    tick = ztick();
    ZDBG("gegin push nocheck");
    list_1r1w_push_nocheck(list, 0, 10000000);
    ZDBG("push down (push nocheck)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_push_nocheck(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_list_pop_nocheck(void* list){
    ztick_t tick;
    int sec,usec;
    zvalue_t val;
    //int i = 0;
    tick = ztick();
    ZDBG("begin pop nocheck");
    while(g_run){
        if(ZOK != zlist_pop(list, &val)){
            zsleepms(0);
        }
    }
    ZDBG("pop down (pop nocheck)");
    while(ZOK == zlist_pop(list, &val));
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop_nocheck(%d): %d.%06d",0, sec, usec);
    return 0;
}

zthr_ret_t ZCALL zproc_list_push(void* list){
    ztick_t tick;
    int sec,usec;
    int i = 1024*1024;
    tick = ztick();
    list_1r1w_push(list, 0, 10000000);
    ZDBG("push down (push)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_list_push(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_list_pop(void* list){
    ztick_t tick;
    int sec,usec, i, old;
    zvalue_t val;
    old = 0;
    tick = ztick();
    while(g_run){
        if(ZOK == zlist_pop(list, &val)){
            ZCONVERT(i, val);
            if(i != (old+1)){
                ZDBG("error val<i:%d old:%d", i , old);
            }
            old = i;
        }
    }
    ZDBG("push down (pop)");
    while(ZOK == zlist_pop(list, &val)){
        ZCONVERT(i, val);
        if(i != (old+1)){
            ZDBG("error val<i:%d old:%d", i , old);
        }
        old = i;
    }
    ZCONVERT(i, val);
    ztock(tick, &sec, &usec);
    ZDBG("zproc_list_pop(%d): %d.%06d",i, sec, usec);
    return 0;
}

static void list_1r1w_thr(zcontainer_t list){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting listue_1r1w_thread now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_list_pop, list);
    zsleepms(10);
    zthread_create(&push_thr,zproc_list_push, list);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting listue_1r1w_thread end\n");
}
static void list_1r1w_thr_nocheck(zcontainer_t list){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting listue_1r1w_thrnocheck now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_list_pop_nocheck, list);
    zsleepms(10);
    zthread_create(&push_thr,zproc_list_push_nocheck, list);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting listue_1r1w_thrnocheck end\n");
}

static void list_1r1w(int argc, char **argv){
    // test listue 1read 1write
    zcontainer_t list;
    zerr_t ret;

    ZDBG("testing listue_1r1w now...");
    ret = zlist_create(&list); ZERRC(ret);
    // base test
    list_1r1w_base(list);
    // 1 read 1 write thread test
    list_1r1w_thr(list);
    list_1r1w_thr_nocheck(list);
    ret = zlist_destroy(list); ZERRC(ret);
    ZDBG("testing listue_1r1w down.");
}
