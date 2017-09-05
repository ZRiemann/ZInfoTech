#ifndef _ZEPOLL_H_
#define _ZEPOLL_H_
/**@file zit/net/epoll.h
 * @brief linux epoll wrapper
 * @note
 *  2017-02-23 ZRiemann found
 * CAUTION:
 * 1)the file description returned by epoll_creat() shoule be closed by close(2)
 * 2)ET only support non-blocked socket, full=>empty empty=>full trigger
 * SAMPLE:
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/thread/thread.h>
#include <zit/thread/spin.h>
#include <zit/container/base/queue.h>
#include <zit/container/map.h>
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
    zvalue_t *hint; /** User define */
    zoperate proc; /** worker threads */
    int workers; /** worker threads */
    /*
     *emq internal
     */
    zthr_id_t thr; /** I/O event thread */
    zsock_t listenfd; /** listen file description */
    zsock_t epfd; /** epoll file description */
    zsock_t pipe[2]; /** control epoll proc exit */
    /* emq containers */
    zmap_t *connections; /** fd to user data */
    zqueue_t *sessions; /** clients buffer */
    /* send/receive buffer allocator */
    zqueue_t buf_que; /** buffer queue */
    int buf_size; /** buffer size */
    zspinlock_t buf_spin; /** spin lock for push buffer */
    /* event message queue  statistic */
    uint32_t total_conns; /** total connections */
    uint64_t total_send; /** total send packets */
    uint64_t total_recv; /** total receive packets */
    uint64_t total_send_hits;
    uint64_t total_recv_hits;
    uint32_t errors;
}zemq_t;

typedef struct zsession_s{
    zsock_t fd;
    zsockaddr_in addr; /** client address */
    char *recv_buf; /** receive buffer */
    char *send_buf; /** send buffer */
    zspinlock_t spin; /** lock parallel send */
    // zatm_t status; /** message ready(1) or idle(0), repleased by recv_hits*/
    zatm32_t recv_hits; /** single thread receive */
    zatm32_t send_hits; /** single thread send (reuse IO thread)*/
    uint64_t send_bytes; /** statistic sended bytes */
    uint64_t recv_bytes; /** statistic receive bytes */
    zvalue_t hint; /** user define data */
}zssn_t;

ZAPI zsock_t zepoll_create();
ZAPI zerr_t zepoll_ctl(int epfd, int op, int fd, struct epoll_event *evt);
ZAPI zerr_t zepoll_wait(int epfd, struct epoll_event *evt, int maxevents, int timeout);

ZAPI zerr_t zemq_init(zemq_t *emq, const char *addr, int32_t port, zvalue_t *hint);
ZAPI zerr_t zemq_fini(zemq_t *emq, zvalue_t *hint);
ZAPI zerr_t zemq_run(zemq_t *emq, zvalue_t *hint);
ZAPI zerr_t zemq_stop(zemq_t *emq, zvalue_t *hint);

#endif /* ZSYS_POSIX */

ZC_END

#endif
