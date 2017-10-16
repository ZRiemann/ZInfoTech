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
#include <zit/framework/task.h>
#include <zit/base/trace.h>
#include <zit/thread/thread.h>

const char *ZTSK_TITLE = "[task] ";

zerr_t zmis_create(zmis_t **mis, int mode){
    zerr_t ret = ZMEM_INSUFFICIENT;
    zmis_t *ms = NULL;
    do{
        if(!mis){
            ret = ZPARAM_INVALID;
            break;
        }
        ms = calloc(1, sizeof(zmis_t));
        if(!ms){
            break;
        }

        if(ZOK != (ret = zqueue_create(&ms->tasks, 4096, sizeof(ztsk_buddy_t)))){
            break;
        }

        zspin_init(&ms->spin_push);
        zspin_init(&ms->spin_stat);
        ms->status = ZMIS_STAT_IDLE;
        ms->mode = mode == ZMIS_MODE_CONCURRENT ? mode : ZMIS_MODE_SERIAL;
        *mis = ms;
        ret = ZOK;
    }while(0);
    if(ZOK != ret){
        free(ms);
    }
    ZERRC(ret);
    return ret;
}

zerr_t zmis_destroy(zmis_t *mis){
    if(mis){
#if 0
        /* Wait mission idle, while unsubscribe dynamically */
        int cnt = 0;
        while(ZMIS_STAT_IDLE != mis->status && cnt < 20){
            zsleepms(100);
            ++cnt;
        }
#endif
        zspin_fini(&mis->spin_push);
        zspin_fini(&mis->spin_stat);
        zqueue_destroy(mis->tasks);
        free(mis);
    }
    return ZOK;
}

static zerr_t ztsk_svr_init(ZOP_ARG){
    return ZOK;
}

static zerr_t ztsk_svr_fini(ZOP_ARG){
    return ZOK;
}

zinline void ztsk_operate(ztsk_t *tsk, zoperate tsk_op, zmis_t *mis, ztsk_svr_t *svr){
    tsk_op((zvalue_t)tsk, (zvalue_t*)mis, (zvalue_t)svr);
    tsk->release((zvalue_t)tsk, NULL, NULL);
}

zinline void ztsk_switch_mis_status(zmis_t *mis, ztsk_svr_t *svr){
    zspin_lock(&mis->spin_stat);
    if(zqueue_size(mis->tasks) > 0){
        /* Mission has more task, Push in mis_ready again. */
        mis->status = ZMIS_STAT_READY;
        //printf("mis<%p, size:%d> push back to mis_ready\n", mis, zqueue_size(mis->tasks));
        zqueue_lock_push(svr->mis_ready, (zvalue_t)&mis, &svr->spin_ready);
        zsem_post(&svr->sem_ready);
    }else{
        /* No more task in mission, set mission idle status. */
        //printf("mis<%p, size:%d> set idle\n", mis, zqueue_size(mis->tasks));
        mis->status = ZMIS_STAT_IDLE;
    }
    zspin_unlock(&mis->spin_stat);
}

static zthr_ret_t ZCALL zproc_tsk_svr(void* param){
    ztsk_svr_t *svr = (ztsk_svr_t*)param;
    zmis_t *mis = NULL;
    zmis_t **ppmis = NULL;
    ztsk_t *tsk = NULL;
    ztsk_buddy_t *tsk_buddy = NULL;
    zoperate tsk_op = NULL;
    int cnt = 0;

    ZDBG("%s task thread<%d> running...", ZTSK_TITLE, zthread_self());

    while(!svr->is_run || *svr->is_run){
        if(ZOK != zsem_wait(&svr->sem_ready, 1000)){
            /* No mission ready */
            continue;
        }

        /* 1. Get the ready mission */
        if(ZOK != zqueue_lock_pop(svr->mis_ready, (zvalue_t*)&ppmis, &svr->spin_ready)){
            ZERRC(ZNOT_EXIST);
            continue;
        }
        mis = *ppmis;

        /* 2. Do mission task and switch mission status */
        if(ZMIS_MODE_CONCURRENT == mis->mode){
            /* 2.1 Concurrent mode mission. */
            if(ZOK == zqueue_front(mis->tasks, (zvalue_t*)&tsk_buddy)){
                tsk = tsk_buddy->tsk;
                tsk_op = tsk_buddy->sub->operate;
                zqueue_pop(mis->tasks, NULL);

                ztsk_switch_mis_status(mis, svr); /* MAST push first */
                ztsk_operate(tsk, tsk_op, mis, svr);
            }else{
                ztsk_switch_mis_status(mis, svr); /* just set idle */
            }
        }else{
            /* 2.2 Serial mode mission */
            for(cnt = 0; cnt < 128; ++cnt){
                /* Max do 128 tasks, avoid mission occupy thread long time. */
                if(ZOK != zqueue_front(mis->tasks, (zvalue_t*)&tsk_buddy)){
                    break;
                }
                ztsk_operate(tsk_buddy->tsk, tsk_buddy->sub->operate, mis, svr);
                zqueue_pop(mis->tasks, NULL);
            }
            ztsk_switch_mis_status(mis, svr);
        } /* if(ZMIS_MODE_CONCURRENT == mis->mode) */
    } /* while(!svr->is_run || *svr->is_run) */

    ZDBG("%s task thread<%d> exit now.", ZTSK_TITLE, zthread_self());

    return 0;
}
static zerr_t ztsk_svr_run(ZOP_ARG){
    zerr_t ret = ZOK;
    ztsk_svr_t *svr = (ztsk_svr_t*)in;
    int i = 0;

    for(i = 0; i < svr->worker_num; ++i){
        if(ZOK != (ret = zthread_create(&svr->workers[i], zproc_tsk_svr, in))){
            break;
        }
    }
    svr->worker_num = i; /* Maybe <i> less then <worker_num> */

    ZERRC(ret);
    return ret;
}

static zerr_t ztsk_svr_stop(ZOP_ARG){
    ztsk_svr_t *svr = (ztsk_svr_t*)in;
    int i = 0;
    int is_run = ZFALSE;

    if(!svr->is_run){
        /* set exit flag */
        svr->is_run = &is_run;
    }

    for(i = 0; i < svr->worker_num; ++i){
        zthread_join(&svr->workers[i]);
    }
    ZERRC(ZOK);
    return ZOK;
}

zerr_t ztsk_svr_create(ztsk_svr_t **tsk_svr, int workers){
    zerr_t ret = ZMEM_INSUFFICIENT;
    ztsk_svr_t *svr = NULL;
    do{
        svr = calloc(1, sizeof(ztsk_svr_t));
        if(!svr){
            break;
        }

        if(ZOK != (ret = zrbtree_create(&svr->subscribers, sizeof(ztsk_subs_t), 32, NULL, ZTRUE))){
            break;
        }
        if(ZOK != (ret = zqueue_create(&svr->mis_ready, 128, (int32_t)sizeof(zqueue_t*)))){
            break;
        }

        if(workers < 2 || workers > 128){
            /* adapter worker numbers */
            workers = 4;
        }
        svr->workers = (zthr_id_t*)calloc(workers, sizeof(zthr_id_t));
        if(!svr->workers){
            ret = ZMEM_INSUFFICIENT;
            break;
        }
        svr->worker_num = workers;
        zsem_init(&svr->sem_ready, 0);
        zspin_init(&svr->spin_ready);
        svr->dev.init = ztsk_svr_init;
        svr->dev.fini = ztsk_svr_fini;
        svr->dev.run = ztsk_svr_run;
        svr->dev.stop = ztsk_svr_stop;
        *tsk_svr = svr;
        ret = ZOK;
    }while(0);

    if(ZOK != ret && svr){
        zqueue_destroy(svr->mis_ready);
        zrbtree_destroy(svr->subscribers);
        free(svr->workers);
        free(svr);
    }
    ZERRCX(ret);
    return ret;
}

zerr_t ztsk_svr_destroy(ztsk_svr_t *svr){
    if(svr){
        zsem_fini(&svr->sem_ready);
        zspin_fini(&svr->spin_ready);
        zqueue_destroy(svr->mis_ready);
        zrbtree_destroy(svr->subscribers);
        free(svr->workers);
        free(svr);
    }
    return ZOK;
}

zerr_t ztsk_svr_subscribe(ztsk_svr_t *svr, ztsk_subs_t *sub){
    zbtnode_t *nod = NULL;
    int ret = ZOK;
    if(ZOK == (ret = zrbtree_get_node(svr->subscribers, &nod))){
        sub->svr = svr;
        zrbtree_set_node(svr->subscribers, nod, sub);
        ret = zrbtree_insert(svr->subscribers, nod);
    }
    ZERRCX(ret);
    return ret;
}

static zerr_t cmp_erase_sub(ZOP_ARG){
    ztsk_subs_t *sub = (ztsk_subs_t*)in;
    ztsk_subs_t *sub1 = (ztsk_subs_t*)hint;
    return (sub->tsk_type == sub1->tsk_type &&
        sub->mis == sub1->mis) ? ZGREAT : ZEQUAL;
}

zerr_t ztsk_svr_unsubscribe(ztsk_svr_t *svr, zotype_t task_type, zmis_t *mis){
    zbtnode_t *nod = NULL;
    ztsk_subs_t *sub = NULL;
    int ret = ZOK;
    if(ZOK == (ret = zrbtree_get_node(svr->subscribers, &nod))){
        /* unsubscribe by task type and mission */
        sub = (ztsk_subs_t*)nod->data;
        sub->tsk_type = task_type;
        sub->mis = mis;
        ret = zrbtree_erasex(svr->subscribers, nod, cmp_erase_sub);
        zrbtree_recycle_node(svr->subscribers, &nod);
    }
    ZERRCX(ret);
    return ret;
}

static zerr_t ztsk_post(ZOP_ARG){
    ztsk_t *tsk = (ztsk_t*)hint;
    zbtnode_t *nod = (zbtnode_t*)in;
    ztsk_subs_t *sub = (ztsk_subs_t*)nod->data;
    zerr_t ret = ZOK;

    if(nod->next && tsk){
        /* Have more subscriber, mast clone the task */
        if(ZOK != (ret = tsk->clone(hint, (zvalue_t*)&tsk, NULL))){
            tsk = (ztsk_t*)hint; /* post last task */
        }
    }/* else  No more subscriber */

    if(tsk){
        ztsk_buddy_t *tsk_buddy = NULL;
        /* Zero memory copy queue buffer */
        if(ZOK == (ret = zqueue_lock_back_pos(sub->mis->tasks,
                                              (zvalue_t*)&tsk_buddy,
                                              &sub->mis->spin_push))){
            tsk_buddy->tsk = tsk;
            tsk_buddy->sub = sub;
            zqueue_unlock_back_pos(sub->mis->tasks, &sub->mis->spin_push);
        }
        /* 2. If mission is idle, post misssion ready semaphore */
        zspin_lock(&sub->mis->spin_stat);
        if(sub->mis->status == ZMIS_STAT_IDLE){
            sub->mis->status = ZMIS_STAT_READY;
            //printf("push mis<%p>\n", sub->mis);
            zqueue_lock_push(sub->svr->mis_ready, (zvalue_t)&sub->mis, &sub->svr->spin_ready);
            zsem_post(&sub->svr->sem_ready);
        }//else{printf("mis is ready<%p>\n", sub->mis);}
        zspin_unlock(&sub->mis->spin_stat);
    }

    ZERRCX(ret);
    return ret;
}

zerr_t ztsk_svr_post(ztsk_svr_t * svr, ztsk_t *tsk){
    /* Post task to each subscriber */
    char buf[128] = {0};
    zbtnode_t *key = (zbtnode_t*)buf;
    ztsk_subs_t *sub = (ztsk_subs_t*)key->data;

    sub->tsk_type = tsk->type;

    if(ZNOT_EXIST == zrbtree_operate(svr->subscribers, key, ztsk_post, tsk, NULL)){
        /* No subscriber on the task */
        ZWAR("%s task<type:0x%x> has no subscriber.", ZTSK_TITLE, tsk->type);
        tsk->release((zvalue_t*)tsk, NULL, NULL);
    }
    return ZOK;
}
