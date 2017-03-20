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
 * 
 */
#include <zit/base/type.h>

ZC_BEGIN

#ifdef ZSYS_WINDOWS
//#error not support windows system
#else //ZSYS_POSIX
#include <zit/net/socket.h>
#include <sys/epoll.h>

ZAPI zsock_t zepoll_create(int size);
ZAPI int zepoll_ctl(int epfd, int op, int fd, struct epoll_event *evt);
ZAPI int zepoll_wait(int epfd, struct epoll_event *evt, int maxevents, int timeout);

#endif

ZC_END
#endif
