/**
 * @file tests/main.cpp
 * @brief contrast c++ lib with libzit;
 */
#include <zit/thread/spin.h>
#include <zit/thread/thread.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>
#include <string.h>
#include <queue>
#include <list>

static int g_run;
static zspinlock_t g_spin;
static void tqueue_1r1w(int argc, char** argv);
static void tlist_1r1w(int argc, char** argv);

int main(int argc, char** argv){
    ztrace_reg(ztrace_console, 0);
    zspin_init(&g_spin);
    if(argc >= 2 && strcmp("queue_1r1w", argv[1]) == 0){
        tqueue_1r1w(argc, argv);
    }else if(argc >= 2 && strcmp("list_1r1w", argv[1]) == 0){
        tlist_1r1w(argc, argv);
    }else{
        zdbg("param error");
    }
    zdbg("all testing down.");
    zspin_fini(&g_spin);
    return 0;
}

/*
 * implement test case
 */
zthr_ret_t ZCALL zproc_push_nocheck(void* pque){
    ztick_t tick;
    int sec,usec;
    int i = 10000000;
    std::queue<int> *que = (std::queue<int>*)pque;
    tick = ztick();
    ZDBG("gegin push nocheck");
    //tqueue_1r1w_push_nocheck(que, 0, 10000000);
    for(int j=0; j<i; ++j){
        zspin_lock(&g_spin);
        que->push(j);
        zspin_unlock(&g_spin);
    }
    ZDBG("push down (push nocheck)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_push_nocheck(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_pop_nocheck(void* pque){
    ztick_t tick;
    int sec,usec;
    std::queue<int> *que = (std::queue<int>*)pque;
    bool empty;

    tick = ztick();
    ZDBG("begin pop nocheck");
    while(g_run){
        zspin_lock(&g_spin);
        empty = que->empty();
        zspin_unlock(&g_spin);

        if(empty){
            zsleepms(0);
        }else{
            zspin_lock(&g_spin);
            que->pop();
            zspin_unlock(&g_spin);
        }
    }
    ZDBG("pop down (pop nocheck)");
    while(!que->empty()){
        que->pop();
    };
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop_nocheck(%d): %d.%06d",0, sec, usec);
    return 0;
}

static void tqueue_1r1w_thr_nocheck(std::queue<int> *que){
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
    std::queue<int> que;
    zdbg("contrast zpin+std::queue with queue_1r1w push/pop 10 million");
    tqueue_1r1w_thr_nocheck(&que);
    zdbg("testing tqueue_1r1w down");
}

//==================================================
// tlist_lr1w


zthr_ret_t ZCALL zproc_list_push_nocheck(void* plist){
    ztick_t tick;
    int sec,usec;
    int i = 10000000;
    std::list<int> *list = (std::list<int>*)plist;
    tick = ztick();
    ZDBG("gegin push nocheck");
    //tlist_1r1w_push_nocheck(list, 0, 10000000);
    for(int j=0; j<i; ++j){
        zspin_lock(&g_spin);
        list->push_back(j);
        if((j & 0x1fffff) == 0){
            zdbg("push back %d", j);
        }
        zspin_unlock(&g_spin);
    }
    ZDBG("push down (push nocheck)");
    g_run = 0;
    ztock(tick, &sec, &usec);
    ZDBG("zproc_push_nocheck(%d): %d.%06d", i, sec, usec);
    return 0;
}
zthr_ret_t ZCALL zproc_list_pop_nocheck(void* plist){
    ztick_t tick;
    int sec,usec;
    std::list<int> *list = (std::list<int>*)plist;
    bool empty;

    tick = ztick();
    ZDBG("begin pop nocheck");
    while(g_run){
        zspin_lock(&g_spin);
        empty = list->empty();
        zspin_unlock(&g_spin);

        if(empty){
            zsleepms(0);
        }else{
            zspin_lock(&g_spin);
            list->pop_front();
            zspin_unlock(&g_spin);
        }
    }
    ZDBG("pop down (pop nocheck)");
    while(!list->empty()){
        list->pop_front();
    };
    ztock(tick, &sec, &usec);
    ZDBG("zproc_pop_nocheck(%d): %d.%06d",0, sec, usec);
    return 0;
}

static void tlist_1r1w_thr_nocheck(std::list<int> *list){
    zthr_id_t push_thr, pop_thr;
    ZDBG("\ntesting list_1r1w_thrnocheck now...\n");
    g_run = 1;
    zthread_create(&pop_thr,zproc_list_pop_nocheck, list);
    zsleepms(10);
    zthread_create(&push_thr,zproc_list_push_nocheck, list);
    zthread_join(&push_thr);
    zthread_join(&pop_thr);
    ZDBG("\ntesting list_1r1w_thrnocheck end\n");
}

static void tlist_1r1w(int argc, char **argv){
    std::list<int> list;
    zdbg("contrast zpin+std::list with list_1r1w push/pop 10 million");
    tlist_1r1w_thr_nocheck(&list);
    zdbg("testing tlist_1r1w down");
}
