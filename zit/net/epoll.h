#ifndef _ZEPOLL_H_
#define _ZEPOLL_H_
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
 * @file zit/net/epoll.h
 * @brief linux epoll wrapper and an event message framework
 * @author 2017-02-23 ZRiemann <Z.Riemann.g(at)gmail.com>
 *
 * @par zemq(z.event message queue framework)
 *   -# implement a session layer;
 *   -# support heart beat;
 *   -# support session timeout;
 *   -# support parallel read/write;
 * @par SAMPLE
 *   see tests/tepoll.h
 * @par to do
 *   -# resume crashed session;
 * @par CAUTION
 *   -# the file description returned by epoll_creat() should be closed by close(2)
 *   -# ET only support non-blocked socket, full=>empty empty=>full trigger
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/spin.h>
#include <zit/container/queue.h>
#include <zit/container/rbtree.h>
#include <zit/container/alloc.h>
#include <zit/base/atomic.h>

ZC_BEGIN

#ifdef ZSYS_WINDOWS
//#error not support windows system
#else //ZSYS_POSIX
#include <zit/net/socket.h>
#include <sys/epoll.h>

typedef struct zioevent_message_queue_s{
    char addr[32];
    uint16_t port;
    zqueue_t *emq; /** zemsg_t queue */
    zspinlock_t spin_in; /** emq in spin */
    zspinlock_t spin_out; /** emq out spin */
    zspinlock_t spin_cmd; /** emq control spin */
    zvalue_t *hint; /** User define */
    /* workers */
    zoperate do_work; /** worker threads */
    zoperate idle_work; /** idle_worker */
    zoperate down_work; /* call before worker thread down */
    zatm32_t idle_status; /** idle_worker 1-busy 0-idle */
    time_t last_idle; /** last idle time */
    time_t last_syn_idle; /** last sync idle work timestamp */
    int worker_num; /** worker threads */
    zthr_id_t *workers; /** worker threads array */
    int *is_run; /** control work exit */
    /* TCP session logic */
    int heart_beat; /** heart beat interval (second), 0 no(default)*/
    int ssn_timeout; /** session time out (second), 0 not time out(default)*/
    zbtree_t *heart_beats; /** control session heart_beat in idle_work */
    /*
     *emq internal
     */
    zthr_id_t thr; /** I/O event thread */
    zsock_t listenfd; /** listen file description */
    zsock_t epfd; /** epoll file description */
    zsock_t pipe[2]; /** control epoll proc exit */
    /* emq containers */
    zbtree_t *sessions; /** session map, fd is key */
    zsem_t sem_emq; /** emq semaphore */
    /* send/receive buffer allocator, set packet max size  */
    zalloc_t *buf_alloc;
    int32_t buf_size; /** buffer size */
    /* event message1 queue  statistic */
    int32_t total_ssns; /** total active sessions */
    uint32_t total_conns; /** total connections, include history conns */
    zatmv64_t total_send; /** total send packets */
    uint64_t total_recv; /** total receive packets */
    uint64_t total_send_hits;
    uint64_t total_recv_hits;
    uint32_t errors;
}zemq_t;

typedef struct zsession_s{
    zsock_t fd; /** as map key */
    uint32_t events; /** EPOLLIN / EPOLLOUT*/
    zsockaddr_in addr; /** client address */
    /* not allocate actual memory buffer, save memory */
    char *packet_buf; /** packet buffer, avoid memory allocate */
    char *recv_buf; /** receive buffer */
    int32_t recv_buf_len; /** receive buffer length*/
    char *send_buf; /** send buffer */
    int32_t send_buf_len; /** send buffer length */
    zspinlock_t spin_send; /** lock parallel send */
    /* status control */
    zatm32_t status; /** IO status message busy(1) or idle(0),
                      *  pull_in check*/
    zatm32_t work_status; /** user define work status */
    zspinlock_t spin_status; /** sessions status change lock */
    /* heart beat control */
    time_t last_operate; /** last receive timestamp */
    time_t heart_beat_stamp; /** next heart beat timestamp,
                              *  key of heart_beat_ssn */
    /* statistic */
    zatm32_t recv_hits; /** single thread receive, worker check */
    zatm32_t send_hits; /** single thread send (reuse IO thread)*/
    zatm32_t pout_hits; /** poll out hits */
    zatm32_t pin_hits; /** poll in hits */
    uint64_t send_bytes; /** statistic sended bytes */
    uint64_t recv_bytes; /** statistic receive bytes */
    char data[]; /** user define data */
}zssn_t;

typedef struct zssn_control_t{
    zssn_t *ssn;
    int op;
    struct epoll_event ev;
}zssn_ctl_t;

#define ZSSN_STAT_READY 1
#define ZSSN_STAT_IDLE 0
#define ZSSN_PACKET_SIZE 1500 /* TCP MTU */

ZAPI zsock_t zepoll_create();
ZAPI zerr_t zepoll_ctl(int epfd, int op, int fd, struct epoll_event *evt);
ZAPI zerr_t zepoll_wait(int epfd, struct epoll_event *evt, int maxevents,
                        int timeout);
ZAPI zerr_t zemq_init(zemq_t *emq, const char *addr, int32_t port,
                      zvalue_t hint, int32_t alloc_buf_size);
ZAPI zerr_t zemq_fini(zemq_t *emq, zvalue_t *hint);
ZAPI zerr_t zemq_run(zemq_t *emq, zvalue_t *hint);
ZAPI zerr_t zemq_stop(zemq_t *emq, zvalue_t *hint);

ZAPI zerr_t zemq_send(zemq_t *emq, zssn_t *ssn, char *buf, int len);
ZAPI zerr_t zemq_ssn_ctl(zemq_t *emq, zssn_ctl_t *ctl,
                         zssn_t *ssn, int op, uint32_t events);
ZAPI zerr_t zemq_hb_format(ZOP_ARG);

ZAPI zerr_t zemq_get_buf(zemq_t *emq, zssn_t *ssn, int is_read);
ZAPI zerr_t zemq_push_buf(zemq_t *emq, zssn_t *ssn, int is_read);

zinline void zemq_clear_buf(zssn_t *ssn, int is_read){
    is_read ? (ssn->recv_buf_len = 0) : (ssn->send_buf_len = 0);
}
#endif /* ZSYS_POSIX */

ZC_END

#endif
