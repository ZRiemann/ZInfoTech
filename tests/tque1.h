#include <zit/container/queue.h>

static void tqueue_1r1w_push(zcontainer_t que, int begin, int end){
    zvalue_t val;
    for(int i = begin; i<end; ++i){
        ZCONVERT(val, i);
        zque1_push(que, val);
    }
}

static void tqueue_1r1w_push_nocheck(zcontainer_t que, int begin, int end){
    //zvalue_t val;
    for(int i = begin; i<end; ++i){
        //ZCONVERT(val, i);
        zque1_push(que, 0);
        /*if( 0 == (i & 0xfffff)){
             ZDBG("push<%d>", i);
        }
        */
    }
}

static void tqueue_1r1w_base(zcontainer_t que){
    zvalue_t val;
    int i;

    ZDBG("dump each value:");
    zque1_foreach(que, dump_int, NULL);
    tqueue_1r1w_push(que, 0, 256);
    zque1_pop(que, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_frontk<%d>\n", i);
    zque1_popback(que, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_back<%d>\n", i);
    ZDBG("\ntesting zque1_foreach(1~254)\n");
    zque1_foreach(que, dump_int, NULL);
    ZDBG("\nbegin pop_fornt:\n");
    while(ZEOK == zque1_pop(que, &val)){
        dump_int(val, NULL, NULL);
    }
    tqueue_1r1w_push(que, 0, 256);
    ZDBG("\nbegin pop_back:\n");
    while(ZEOK == zque1_popback(que, &val)){
        dump_int(val, NULL, NULL);
    }
}

zthr_ret_t ZCALL zproc_push_nocheck(void* que){
    ztick_t tick;
    int sec,usec;
    int i = 1024*1024;
    tick = ztick();
    ZDBG("gegin push nocheck");
    tqueue_1r1w_push_nocheck(que, 0, 10000000);
    ZDBG("push down (push nocheck)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_push_nocheck(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_pop_nocheck(void* que){
    ztick_t tick;
    int sec,usec;
    zvalue_t val;
    //int i = 0;
    tick = ztick();
    ZDBG("begin pop nocheck");
    while(g_run){
        if(ZOK != zque1_pop(que, &val)){
            zsleepms(0);
        }
    }
    ZDBG("pop down (pop nocheck)");
    while(ZOK == zque1_pop(que, &val));
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop_nocheck(%d): %d.%06d",0, sec, usec);
    return 0;
}

zthr_ret_t ZCALL zproc_push(void* que){
    ztick_t tick;
    int sec,usec;
    int i = 1024*1024;
    tick = ztick();
    tqueue_1r1w_push(que, 0, 10000000);
    ZDBG("push down (push)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_push(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_pop(void* que){
    ztick_t tick;
    int sec,usec, i, old;
    zvalue_t val;
    old = 0;
    tick = ztick();
    while(g_run){
        if(ZOK == zque1_pop(que, &val)){
            ZCONVERT(i, val);
            if(i != (old+1)){
                ZDBG("error val<i:%d old:%d", i , old);
            }
            old = i;
        }
    }
    ZDBG("push down (pop)");
    while(ZOK == zque1_pop(que, &val)){
        ZCONVERT(i, val);
        if(i != (old+1)){
            ZDBG("error val<i:%d old:%d", i , old);
        }
        old = i;
    }
    ZCONVERT(i, val);
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop(%d): %d.%06d",i, sec, usec);
    return 0;
}

static void tqueue_1r1w_thr(zcontainer_t que){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting queue_1r1w_thread now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_pop, que);
    zsleepms(10);
    zthread_create(&push_thr,zproc_push, que);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting queue_1r1w_thread end\n");
}
static void tqueue_1r1w_thr_nocheck(zcontainer_t que){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting queue_1r1w_thrnocheck now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_pop_nocheck, que);
    zsleepms(10);
    zthread_create(&push_thr,zproc_push_nocheck, que);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting queue_1r1w_thrnocheck end\n");
}

static void tqueue_1r1w(int argc, char **argv){
    // test queue 1read 1write
    zcontainer_t que;
    zerr_t ret;

    ZDBG("testing queue_1r1w now...");
    ret = zque1_create(&que); ZERRC(ret);
    // base test
    tqueue_1r1w_base(que);
    // 1 read 1 write thread test
    tqueue_1r1w_thr(que);
    tqueue_1r1w_thr_nocheck(que);
    ret = zque1_destroy(que); ZERRC(ret);
    ZDBG("testing queue_1r1w down.");
}

