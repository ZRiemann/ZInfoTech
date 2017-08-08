#include <zit/container/container.h>

static void tcontainer_push(zcontainer_t cont, int begin, int end){
    zvalue_t val;
    for(int i = begin; i<end; ++i){
        ZCONVERT(val, i);
        zcontainer_push(cont, val);
    }
}

static void tcontainer_push_nocheck(zcontainer_t cont, int begin, int end){
    for(int i = begin; i<end; ++i){
        zcontainer_push(cont, 0);
    }
}


static zerr_t tcont_compare(ZOP_ARG){
	int a, b;
	int sub;
	int *cmp = (int*)out;

	ZCONVERT(a, hint);
	ZCONVERT(b, in);
	sub = a - b;
    sub = sub ? (sub > 0 ? ZGREAT : ZLITTLE) : ZEQUAL;
    *cmp = sub;
    ZDBG("hint: %d in: %d cmp: %d", hint, in, sub);
    return ZOK;
}

static void tcontainer_base(zcontainer_t cont){
    zvalue_t val;
    int i;

    ZDBG("dump each value:");
    zcontainer_foreach(cont, dump_int, NULL);
    tcontainer_push(cont, 0, 256);
    zcontainer_pop(cont, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_frontk<%d>\n", i);
    zcontainer_popback(cont, &val);
    ZCONVERT(i,val);
    ZDBG("\npop_back<%d>\n", i);
    ZDBG("\ntesting zcontainer_foreach(1~254)\n");
    zcontainer_foreach(cont, dump_int, NULL);
    ZDBG("\nbegin pop_fornt:\n");
    while(ZEOK == zcontainer_pop(cont, &val)){
        dump_int(val, NULL, NULL);
    }
    tcontainer_push(cont, 0, 10);
    i = 3;
    ZCONVERT(val, i);
    zcontainer_insert(cont, val, tcont_compare, ZLITTLE);
    ZDBG("\ndump insert 3:\n");
    zcontainer_foreach(cont, dump_int, NULL);
    i = 4;
    ZCONVERT(val, i);
    zcontainer_erase(cont, val, tcont_compare, ZEQUAL);
    ZDBG("\ndump earse EQUAL 4:\n");
    zcontainer_foreach(cont, dump_int, NULL);
    i = 5;
    ZCONVERT(val, i);
    zcontainer_erase(cont, val, tcont_compare, ZGREAT);
    ZDBG("\ndump earse ZGREAT 4:\n");
    zcontainer_foreach(cont, dump_int, NULL);

    ZDBG("\nbegin pop_back:\n");
    while(ZEOK == zcontainer_popback(cont, &val)){
        dump_int(val, NULL, NULL);
    }
}

zthr_ret_t ZCALL zproc_cont_push_nocheck(void* cont){
    //ztick_t tick;
    //int sec,usec;
    //tick = ztick();
    //ZDBG("gegin push nocheck");
    tcontainer_push_nocheck(cont, 0, 10000000);
    //ZDBG("push down (push nocheck)");
    g_run = 0;
    //ztock(tick, &sec, &usec);
    //ZDBG("zproc_push_nocheck(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_cont_pop_nocheck(void* cont){
    ztick_t tick;
    int sec,usec;
    zvalue_t val;
    //int i = 0;
    tick = ztick();
    //ZDBG("begin pop nocheck");
    while(g_run){
        if(ZOK != zcontainer_pop(cont, &val)){
            zsleepms(0);
        }
    }
    //ZDBG("pop down (pop nocheck)");
    while(ZOK == zcontainer_pop(cont, &val));
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop_nocheck_COST: %d.%06d",sec, usec);
    return 0;
}

zthr_ret_t ZCALL zproc_cont_push(void* cont){
    //ztick_t tick;
    //int sec,usec;
    //int i = 1024*1024;
    //tick = ztick();
    tcontainer_push(cont, 0, 10000000);
    //ZDBG("push down (push)");
    g_run = 0;
    //ztock(tick, &sec, &usec);
    //ZDBG("zproc_cont_push(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_cont_pop(void* cont){
    ztick_t tick;
    int sec,usec, i, old;
    zvalue_t val;
    old = 0;
    tick = ztick();
    while(g_run){
        if(ZOK == zcontainer_pop(cont, &val)){
            ZCONVERT(i, val);
            if(i != (old+1)){
                ZDBG("error val<i:%d old:%d", i , old);
            }
            old = i;
        }
    }
    //ZDBG("push down (pop)");
    while(ZOK == zcontainer_pop(cont, &val)){
        ZCONVERT(i, val);
        if(i != (old+1)){
            ZDBG("error val<i:%d old:%d", i , old);
        }
        old = i;
    }
    ZCONVERT(i, val);
    ztock(tick, &sec, &usec);
    ZDBG("zproc_cont_popCHECK(%d): %d.%06d",i, sec, usec);
    return 0;
}

static void tcontainer_thr(zcontainer_t cont){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting contue_1r1w_thread now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_cont_pop, cont);
    zsleepms(10);
    zthread_create(&push_thr,zproc_cont_push, cont);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting contue_1r1w_thread end\n");
}
static void tcontainer_thr_nocheck(zcontainer_t cont){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting contue_1r1w_thrnocheck now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_cont_pop_nocheck, cont);
    zsleepms(10);
    zthread_create(&push_thr,zproc_cont_push_nocheck, cont);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting contue_1r1w_thrnocheck end\n");
}

static void tcontainer(int argc, char **argv){
    // test container 1read 1write
    zcontainer_t cont;
    zerr_t ret;

    for(int i= 0; i<10; i++){
        ZDBG("testing container<%d> now...", i);
        ret = zcontainer_create(&cont, i);
        if(ZOK != ret){
            ZDBG("container<%d> %s", i, zstrerr(ret));
            continue;
        }
        // base test
        tcontainer_base(cont);
        // 1 read 1 write thread test
        tcontainer_thr(cont);
        tcontainer_thr_nocheck(cont);
        ret = zcontainer_destroy(cont); ZERRC(ret);
        ZDBG("testing cont<%d> down.", i);
    }
}
