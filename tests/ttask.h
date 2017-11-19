#ifndef _ZTST_TASK_H_
#define _ZTST_TASK_H_

#include <zit/memory/obj_pool.h>
#include <zit/framework/task.h>

#define ZTSK_TYPE_MSG 0x00000001
#define ZTSK_TYPE_THROUGHPUT 0x00000002

typedef struct ztask_message_s{
    ztsk_t tsk;
    char msg[64];
}ztsk_msg_t;

static zerr_t ttask_print_msg(ZOP_ARG){
    ztsk_msg_t *tsk = (ztsk_msg_t*)in;
    printf("tid<%d> print msg: %s\n", zthread_self(), tsk->msg);
    return ZOK;
}

static zerr_t ttask_trace_msg(ZOP_ARG){
    ztsk_msg_t *tsk = (ztsk_msg_t*)in;
    ZDBG("tid<%d> trace msg: %s", zthread_self(), tsk->msg);
    return ZOK;
}

static zerr_t ttask_throughput_serial(ZOP_ARG){
    static int old = 0;
    ztsk_msg_t *tsk = (ztsk_msg_t*)in;
    int *serial_num = (int*)tsk->msg;
    if(old && (old+1 != *serial_num)){
        zdbg("ttask_throughput_serial(tsk<ptr:%p,serial_num:%d> old:%d)",tsk, *serial_num, old);
    }
    old = *serial_num;
    if(!(*serial_num & 0xfffff)){
        ZDBG("tid<%d> mis_serial<serial_num:%d>", zthread_self(), *serial_num);
    }
    return ZOK;
}

static zerr_t ttask_throughput_concurrent(ZOP_ARG){
    static int old = 0;
    ztsk_msg_t *tsk = (ztsk_msg_t*)in;
    int *serial_num = (int*)tsk->msg;

    if(old && (old+1 != *serial_num)){
        /* concurrent task number is not serial */
        //printf("concurrent task serial:%d old:%d\n", *serial_num, old);
    }
    old = *serial_num;

    if(!(*serial_num & 0xfffff)){
        ZDBG("tid<%d> mis_concurrent<serial_num:%d>", zthread_self(), *serial_num);
    }
    return ZOK;
}

static void ttask_subscribe(ztsk_svr_t *tsk_svr, zmis_t *mis_serial,
                            zmis_t *mis_concurrent){
    ztsk_subs_t sub;
    sub.tsk_type = ZTSK_TYPE_MSG;
    sub.mis = mis_serial;
    sub.operate = ttask_print_msg;
    ztsk_svr_subscribe(tsk_svr, &sub);
    ztsk_svr_subscribe(tsk_svr, &sub); /* multiple subscribe */

    sub.mis = mis_concurrent;
    sub.operate = ttask_trace_msg;
    ztsk_svr_subscribe(tsk_svr, &sub);

    /* subscribe throughput task */
    sub.tsk_type = ZTSK_TYPE_THROUGHPUT;
    sub.mis = mis_serial;
    sub.operate = ttask_throughput_serial;
    ztsk_svr_subscribe(tsk_svr, &sub);

    sub.tsk_type = ZTSK_TYPE_THROUGHPUT;
    sub.mis = mis_concurrent;
    sub.operate = ttask_throughput_concurrent;
    //ztsk_svr_subscribe(tsk_svr, &sub);
}

static void ttask_generate(ztsk_svr_t *tsk_svr, zobj_pool_t *pool,
                           zotype_t tsk_type, int total){
    zerr_t ret = ZOK;
    int i = 0;
    int interval = 1000; /* ms */
    ztsk_t *tsk = NULL;

    if(ZTSK_TYPE_MSG == tsk_type){
        for(i = 0; i < total; ++i){
            interval -= 100;
            if(interval < 10){
                interval = 1;
            }
            zsleepms(interval);
            if(ZOK == (ret = zobj_pool_pop(pool, &tsk, ZCLONE_MODE_REF))){
                tsk->type = tsk_type;
                printf("post msg: message[%d]\n", i);
                sprintf(zobj_extern((char*)tsk), "message[%d]", i);
                ztsk_svr_post(tsk_svr, tsk);
            }else{
                ZERRC(ret);
            }
        }
    }else if(ZTSK_TYPE_THROUGHPUT == tsk_type){
        /* statistic task framework throughput */
        for(i = 0; i < total; ++i){
            if(ZOK == (ret = zobj_pool_pop(pool, &tsk, ZCLONE_MODE_REF))){
                tsk->type = tsk_type;
                /* *(int*)tsk->data = i; */
				*(int*)zobj_extern((char*)tsk);
                if(!(i & 0xfffff)){
                    ZDBG("post throughput task<%d>", i);
                }
                //zsleepms(0);
                ztsk_svr_post(tsk_svr, (ztsk_t*)tsk);
            }else{
                ZERRC(ret);
                --i;
                zsleepms(200);
            }
        }
    }
}

static void ttask(int argc, char **argv){
    ztsk_svr_t *tsk_svr = NULL;
    zmis_t *mis_serial = NULL;
    zmis_t *mis_concurrent = NULL;
    zobj_pool_t *pool = NULL;
    int tsk_num = 10;
    zotype_t tsk_type = ZTSK_TYPE_MSG;

    if(argc >= 4){
        /* Set task number */
        tsk_num = atoi(argv[3]);
        if(tsk_num < 10){
            tsk_num = 10;
        }

        /* Set task type */
        if(0 == strcmp("throughput", argv[2])){
            tsk_type = ZTSK_TYPE_THROUGHPUT;
            if(tsk_num < 0xfffff){
                tsk_num = 0x200001;
            }
        }else{
            tsk_type = ZTSK_TYPE_MSG;
        }
    }

    ZDBG("Begin test task server....");

    ztsk_svr_create(&tsk_svr, 0);
    tsk_svr->dev.init((zvalue_t)tsk_svr, NULL, NULL);
    tsk_svr->dev.run((zvalue_t)tsk_svr, NULL, NULL);
    zsleepms(200);

    zmis_create(&mis_serial, ZMIS_MODE_SERIAL);
    zmis_create(&mis_concurrent, ZMIS_MODE_CONCURRENT);

    zobj_pool_create(&pool, sizeof(ztsk_msg_t), 4096, 0);

    ttask_subscribe(tsk_svr, mis_serial, mis_concurrent);
    ttask_generate(tsk_svr, pool, tsk_type, tsk_num);

    zsleepms(500);

    tsk_svr->dev.stop((zvalue_t)tsk_svr, NULL, NULL);
    tsk_svr->dev.fini((zvalue_t)tsk_svr, NULL, NULL);

    zmis_destroy(mis_serial);
    zmis_destroy(mis_concurrent);

    zobj_pool_destroy(pool);

    ZDBG("End test task server.");
}
#endif
