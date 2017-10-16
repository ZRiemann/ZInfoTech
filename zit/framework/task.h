#ifndef _ZFR_TASK_H_
#define _ZFR_TASK_H_
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
/**
 * @file zit/framework/task.h
 * @brief zit task definitions
 * @author Z.Riemann <Z.Riemann.g@gmail.com>
 * @date 2016-9-12 Z.Riemann found
 *
 * @par design pattern
 *      -# Coupling-separation task generator and handler;
 *         Task handler need create a mission and
 *         implement a operate function, before subscribe task type;
 *      -# Support object memory manager;
 *      -# Dynamic add and remove task handler and generator;
 *
 * @par overview
 *      -# ztsk_svr_t: task server;
 *           ztsk_svr_create()
 *           ztsk_svr_destroy()
 *           ztsk_svr_subscribe()
 *           ztsk_svr_unsubscribe()
 *           ztsk_svr_post()
 *      -# zmis_t: mission a task queue;
 *           zmis_create()
 *           zmis_destroy()
 *      -# ztsk_alloc_t: task allocator;
 *           ztsk_alloc_create()
 *           ztsk_alloc_destroy()
 *           ztsk_alloc()
 *      -# ztsk_subs_t: task subscriber;
 *      -# ztsk_t: task abstrace;
 *
 * @par useage
 *      -# sample code at ZInfoTech/tests/ttask.h
 *
 *      -# In main thread, create and run task server
 *         #include <zit/framework/task.h>
 *         ztsk_svr_t *tsk_svr = NULL;
 *         ztsk_svr_create(&tsk_svr, 4);
 *         tsk_svr->dev.init((zvalue_t)tsk_svr, NULL, NULL);
 *         tsk_svr->dev.run((zvalue_t)tsk_svr, NULL, NULL);
 *
 *         wait exit condition ...
 *
 *         tsk_svr->dev.stop((zvalue_t)tsk_svr, NULL, NULL);
 *         tsk_svr->dev.fini((zvalue_t)tsk_svr, NULL, NULL);
 *         ztsk_svr_destroy(tsk_svr);
 *
 *      -# Task generator
 *         #include <zit/framework/task.h>
 *         #define ZTSK_TYPE_MSG 0x00000001
 *
 *         typedef struct ztask_message_s{
 *             ztsk_t tsk;
 *             char msg[128];
 *         }ztsk_msg_t;
 *
 *         ztsk_alloc_t *alloc = NULL;
 *         ztsk_alloc_create(&alloc, sizeof(ztsk_msg_t), 128, 32*1024*1024);
 *
 *         ztsk_msg_t *tsk = NULL;
 *         ztsk_alloc(alloc, &tsk);
 *         sprintf(tsk->msg, "hello world!\n");
 *
 *         ztsk_svr_post(tsk_svr, (ztsk_t*)tsk);
 *
 *      -# Task handler
 *         #include <zit/framework/task.h>
 *         static zerr_t print_msg(ZOP_ARG){
 *             ztsk_msg_t *tsk = (ztsk_msg_t*)in;
 *             printf("handle task: %s", tsk->msg);
 *             return ZOK;
 *         }
 *
 *         zmis_t *mis = NULL;
 *         zmis_create(&mis, ZMIS_MODE_SERIAL);
 *
 *         ztsk_subs_t sub;
 *         sub.tsk_type = ZTSK_TYPE_MSG;
 *         sub.mis = mis;
 *         sub.operate = print_msg;
 *         ztsk_svr_subscribe(tsk_svr, &sub);
 *
 * @par Performance
 *      -# Throughput
 *         about 1,500,000 task per seconds, by 1 serial mission.
 *         see task case:
 *         make/release/zit_test task throughput 0
 *
 *         10:17:39.848 post throughput task<0>
 *         10:17:40.623 post throughput task<1048576>
 *         10:17:41.385 post throughput task<2097152>
 *
 *         10:17:39.848 mis_serial<serial_num:0>
 *         10:17:40.623 mis_serial<serial_num:1048576>
 *         10:17:41.385 mis_serial<serial_num:2097152>
 *
 *         about 350,000 task per seconds, by 2 mission subscribe 1 task.
 */
#include <zit/framework/object.h>
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <zit/memory/alloc.h>
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/spin.h>
#include <zit/container/queue.h>
#include <zit/container/rbtree.h>

#define ZMIS_MODE_SERIAL 0
#define ZMIS_MODE_CONCURRENT 1
#define ZMIS_STAT_IDLE 0
#define ZMIS_STAT_READY 1

/**
 * @brief Task server definition
 */
typedef struct ztask_server_s{
    zdev_t dev; /** internal implement init/fini/run/stop */
    zbtree_t *subscribers; /** multi_map<zotype_t, ztsk_subs_t> > */

    /* mission queue */
    zqueue_t *mis_ready; /** mission ready */
    zsem_t sem_ready; /** mission ready semaphore */
    zspinlock_t spin_ready; /** lock for push mish_ready */

    /* thread pool */
    zthr_id_t *workers; /** zthr_id_t array */
    int worker_num; /** work numbers */
    int *is_run; /** global exit flag */
}ztsk_svr_t;

/**
 * @brief general task definition
 */
typedef zobj_t ztsk_t;

/**
 * @brief Task queue
 */
typedef struct zmission_s{
    int mode; /** ZMIS_MODE_SERIAL/CONCURRENT */
    zqueue_t *tasks; /** queue<ztsk_buddy_t> */
    zspinlock_t spin_push; /** push queue lock */
    zspinlock_t spin_stat; /** switch mission status lock */
    int32_t status; /** ZMIS_STAT_IDLE/READY */
}zmis_t;

/**
 * @brief Task subscriber
 */
typedef struct ztask_subscriber_s{
    zotype_t tsk_type; /** subscribe task type */
    ztsk_svr_t *svr; /** pointer to task server */
    zmis_t *mis; /** pointer to mission*/
    zoperate operate; /** operate on task */
}ztsk_subs_t;

/**
 * @brief task buddy
 *
 *  Increase of efficiency while multiple subscribers on same task.
 *  Avoid memory copy by use reference count.
 */
typedef struct ztask_buddy_s{
    ztsk_t *tsk; /** task pointer */
    ztsk_subs_t *sub; /** task subscriber pointer */
}ztsk_buddy_t;

/**
 * @brief create a mission with mode
 * @param [out] mis pointer to mission pointer
 * @param [in] mode serial or concurrent mode
 * @return ZOK success
 * @return ZMEM_INSUFFICIENT memory insufficient
 * @return ZPARAM_INVALID parameter invalid
 */
ZAPI zerr_t zmis_create(zmis_t **mis, int mode);

/**
 * @brief destroy a mission, create by zmis_create()
 * @param [in] mis pointer to mission
 * @return ZOK success
 */
ZAPI zerr_t zmis_destroy(zmis_t *mis);

/**
 * @brief create a task server
 * @param [out] tsk_svr point to task server pointer
 * @param [in] workers thread poll size
 * @param ZOK success
 * @param ZMEM_INSUFFICIENT not enough memory
 */
ZAPI zerr_t ztsk_svr_create(ztsk_svr_t **tsk_svr, int workers);

/**
 * @brief destroy a task server, create by ztsk_svr_create()
 * @param [in] task server pointer
 * @return ZOK success
 */
ZAPI zerr_t ztsk_svr_destroy(ztsk_svr_t *tsk_svr);

/**
 * @brief subscribe a task category
 * @param [in] svr task server pointer
 * @param [in] sub subscriber pointer
 */
ZAPI zerr_t ztsk_svr_subscribe(ztsk_svr_t *svr, ztsk_subs_t *sub);
/**
 * @brief unsubscribe a task category
 * @param [in] svr task server pointer
 * @param [in] task_type task category
 * @param [in] mis mission pointer
 * @return ZOK success
 * @return ZNOT_EXIST no subscriber on task_type
 */
ZAPI zerr_t ztsk_svr_unsubscribe(ztsk_svr_t *svr, zotype_t task_type,
                                 zmis_t *mis);
/**
 * @brief post a task to task server
 * @param [in] svr task server pointer
 * @param [in] tsk task pointer
 */
ZAPI zerr_t ztsk_svr_post(ztsk_svr_t * svr, ztsk_t *tsk);

#endif
