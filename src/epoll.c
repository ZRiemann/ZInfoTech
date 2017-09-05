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
 * @file epoll.c
 * @brief zemq implements
 *
 * @pra Design module
 *  -# stop control by socket close
 *  -# Buffer module
 *     - Ring buffer;
 *     - set socket send/receive buffer;
 *  -# One thread IO avoid mulit thread parallel send or receive;
 *  -# epoll thread just post the event message, send or receive use;
 */
#include "export.h"
#include <zit/net/epoll.h>
#include <zit/base/time.h>

#ifdef ZSYS_POSIX
#define ZTRACE_EPOLL 0

#include <zit/base/trace.h>
#include <arpa/inet.h>
#include <unistd.h>

zsock_t zepoll_create(){
  zsock_t fd = epoll_create1(EPOLL_CLOEXEC);
  if(-1 == fd){
    ZERRC(errno);
  }
#ifdef ZTRACE_EPOLL
  ZDBG("create epoll fd<%d>", fd);
#endif
  return(fd);
}

zerr_t zepoll_ctl(int epfd, int op, int fd, struct epoll_event *evt){
  int ret;
#ifdef ZTRACE_EPOLL
  ZDBG("epoll_%s<epfd:%d, fd:%d,ev.events:0x%x>", op==EPOLL_CTL_ADD?"ADD":op==EPOLL_CTL_MOD?"MOD":"DEL", epfd, fd, evt->events);
#endif
  ret = epoll_ctl(epfd, op, fd, evt);
  if(ZOK != ret){
    if(ret == EEXIST){
      ret = ZOK;
    }else{
      ret = ZFUN_FAIL;
    }
    ZERRC(errno);
  }
  return ret;
}

zerr_t zepoll_wait(int epfd, struct epoll_event *evt, int maxevents, int timeout){
  int ret;
  ret = epoll_wait(epfd, evt, maxevents, timeout);
  if(-1 == ret){
    ZERRC(errno);
  }
  return ret;
}

zerr_t zemq_init(zemq_t *emq, const char *addr, int32_t port, zvalue_t *hint){
    if(!emq){
        return ZPARAM_INVALID;
    }
    memset(emq, 0, sizeof(zemq_t));
    emq->hint = hint;
    emq->port = port;
    if(!addr){
        addr = "0.0.0.0";
    }
    sprintf(emq->addr, "%s", addr);
    zqueue_create(&emq->emq, 4096, (int32_t)sizeof(zvalue_t));
    zqueue_create(&emq->sessions, 1024, (int32_t)sizeof(zssn_t));
    zmap_create(&emq->connections, 64);
#ifdef ZTRACE_EPOLL
    ZERRC(ZOK);
#endif
    return ZOK;
}
zerr_t zemq_fini(zemq_t *emq, zvalue_t *hint){
    /* Destroy all resource... */
    zqueue_destroy(emq->emq);
    zmap_destroy(emq->connections);
#ifdef ZTRACE_EPOLL
    ZERRC(ZOK);
#endif
    return ZOK;
}

#define ZMAX_EVENTS 16
#define ZECHO_TEST 1
#define ZECHO_BUF_SIZE 256

#if 0
zinline zerr_t zbuild_emq(zemq_t *emq, int *epfd, int *listenfd){
    zerr_t ret = ZOK;
    struct epoll_event ev = {0};
    do{
        /* Create a listen socket and set nonblock */
        if(ZINVALID_SOCKET == (emq->listenfd = zsocket(AF_INET, SOCK_STREAM, 0))){
            ret = ZFUN_FAIL;
            break;
        }
        /* Bind and listen */
        if(ZOK != (ret = zconnectx(emq->listenfd, emq->addr, emq->port, 256, 0)))break;

        if(-1 == (emq->epfd = zepoll_create())){
            ret = ZFUN_FAIL;
            break;
        }
        /* add listen fd to epoll */
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.ptr = &emq->listenfd;
        ret = zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, emq->listenfd, &ev);
        /* add cmd pipe to epoll */
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.ptr = &emq->pipe[0];
        ret = zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, emq->pipe[0], &ev);

        *epfd = emq->epfd;
        *listenfd = emq->listenfd;
        *exitfd = emq->emq->pipe[0];
    }while(0);

    return ret;
}

zinline void zclose_fds(zssn_t *ssns, int size){
    for(int i = 0; i < size; ++i){
        zsockclose(ssns[i]->fd);
    }
}

zinline void zaccept_session(zemq_t *emq){
    struct epoll_event ev = {0};
    zssn_t *ssn = NULL;
    int len;
    int ret;
    for(;;){
        if(ZOK != (ret = zqueue_back_pos(emq->sessions, (zvalue_t*)&ssn))){
            break;
        }
        len = sizeof(zsockaddr_in);
        if(0 > (ssn->fd = accept(emq->listenfd, &ssn->addr, &len))){
            zqueue_pop_back(emq->sessions, NULL);
            if(errno == ECONNABORTED || errno == EPROTO || errno == EINTR){
                ZDBG("May be more connection, error: %s", strerror(errno));
                continue;
            }else{
                break; /* No more connection */
            }
        }
        zsock_nonblock(ssn->fd, 1);
        ev.events = ZEPOLLIN | EPOLLET;
        ev.data.ptr = ssn;
        if(ZOK != zepoll_ctl(qme->epfd, EPOLL_CTL_ADD), ssn->fd, &ev){
            zsockclose(ssn->fd);
            zqueue_pop_back(emq->session, NULL);
        }else{
            ++emq->total_conns;
        }
    }
    ZERRCX(ret);
}

static zthr_ret_t ZCALL zproc_emq(void* param){
    zemq_t *emq = (zemq_t*)param;
    struct epoll_event ev = {0};
    struct epoll_event events[ZMAX_EVENTS] = {0};
    zsock_t listenfd, connfd, epfd, curfd, exitfd;
    int32_t idx, ret, nfds, len;
    zssn_t ssns[ZMAX_EVENTS] = {0};
    int del_size = 0;
    int is_run = 1;
    if(ZOK != zbuild_emq(emq, &epfd, &listenfd, &exitfd)){
        return 0;
    }

    while(is_run){
        del_size = 0;
        if(-1 == (nfds = epoll_wiat(epfd, events, ZMAX_EVENTS, -1))){
            ZERRC(errno);
            return 0;
        }
        for(idx = 0; idx < nfds; ++idx){
            curfd = *((zsock_t*)(events[idx].data.ptr));
            if(curfd == exitfd){
                /* exit command in*/
                is_run = 0;
            }else if(curfd == listenfd){
                /* accept new session */
                zaccept_session(emq);
            }else if(events[idx].events & EPOLLIN){
                /* in buffer notify */
                zepoll_in();
            }else if(events[idx].events & EPOLLOUT){
                /* out bffer notify */
                zepoll_out();
            }else{
                /* some other events */
                ZWAR("not support events<%d>", events[idx].events);
            }
        }/*for idx */
        zclose_fds(del_fds, del_size);
    }/* while is run */
}
#else
static zthr_ret_t ZCALL zproc_emq(void* param){
    zemq_t *emq = (zemq_t*)param;
    zsockaddr_in listen_addr = {0};
    zsock_t listenfd = -1;
    zsock_t connfd = -1;
    zsock_t epfd = -1;
    zsock_t fd = -1;
    zerr_t ret = ZOK;
    int nfds = 0;
    int32_t idx = 0;
    int len = 0;
    struct epoll_event ev = {0};
    struct epoll_event events[ZMAX_EVENTS] = {0};
    int del_fds[ZMAX_EVENTS];
    int del_size;
    uint64_t echo_num = 0;
    int is_run = 1;
#if ZTRACE_EPOLL
    /* client */
    char host[64];
    uint16_t port;
#endif

#if ZECHO_TEST
    char echo_buf[ZECHO_BUF_SIZE];
    int nread;
#endif
    do{
        /* Create a listen socket and set nonblock */
        if(ZINVALID_SOCKET == (listenfd = zsocket(AF_INET, SOCK_STREAM, 0))){
            ret = ZFUN_FAIL;
            break;
        }
        if(ZOK != (ret = zconnectx(listenfd, emq->addr, emq->port, 256, 0)))break;
        emq->listenfd = listenfd;

        if(-1 == (epfd = zepoll_create())){
            ret = ZFUN_FAIL;
            break;
        }
        emq->epfd = epfd;
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = listenfd;
        ret = zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, listenfd, &ev);

        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = emq->pipe[0];
        ret = zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, emq->pipe[0], &ev);
    }while(0);

    if(ZOK != ret){
        return (zthr_ret_t)-1;
    }

    while(is_run){
        del_size = 0;
        nfds = epoll_wait(epfd, events, ZMAX_EVENTS, -1);
        if(-1 == nfds){
            ZERRC(errno);
            return (zthr_ret_t)-1;
        }

        for(idx=0; idx<nfds; ++idx){
            fd = events[idx].data.fd;
            //ZDBG("fd: %d", fd);
            if(fd == listenfd){
                /* Get new connections */
                while(1){
                    if(0 > (connfd = zaccept(listenfd, (ZSA*)&listen_addr, &len))){
                        if(errno == ECONNABORTED || errno == EPROTO || errno == EINTR){

                            continue; /* May be has more connection */
                        }else{
                            ZDBG("No more connections in queue. error:%d %s", errno, strerror(errno));
                            break; /* No more connection */
                        }
                    }
                    zsock_nonblock(connfd, 1);
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = connfd;
                    if(ZOK != zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, connfd, &ev)){
                        ZERRC(ZFUN_FAIL);
                        zsockclose(connfd);
                    }else{
#if ZTRACE_EPOLL
                        zinet_str(&listen_addr, host, &port);
                        ZDBG("accept connection<fd:%d addr: %s:%d>", connfd, host, port);
#endif
                        ++emq->total_conns;
                        ZDBG("accept total_fds:%d", emq->total_conns);
                    }
                }
            }else if(fd == emq->pipe[0]){
                ZMSG("Stop epoll proc ....");
                is_run = 0; /* control exit*/
            }else if(events[idx].events & EPOLLIN){
                /* socket in buffer not empty or change notify */
#if ZECHO_TEST
                /* echo server test */
                while(1){ /* may be continue by EINTR */
#if ZTRACE_EPOLL
                    ZDBG("event[%d]<fd: %d> EPOLLIN", idx, fd);
#endif
                    if(0 < (nread = recv(fd, echo_buf, ZECHO_BUF_SIZE, 0))){//read(fd, echo_buf, ZECHO_BUF_SIZE))){
                        nread = send(fd, echo_buf, nread, 0); /* ignore write < read for simple test case */
#if ZTRACE_EPOLL
                        //ZDBG("receive nread<%d> error:%s", nread, strerror(errno));
#endif
                        if((++echo_num & 0xfffff) == 0){
                            ZDBG("echo_num: %llu", (long long unsigned int)echo_num);
                        }
                    }
                    if(0 == nread){
                        /* Connection gracefully closed by peer. */
                        ZMSG("Connection<%d> gracefully closed by peer.", fd);
                        del_fds[del_size++] = fd;
                        break;
                    }else if(errno == EAGAIN ){
                        /* In buffer is empty */
                        break;
                    }else if(errno == EINTR){
                        /* Try again. */
                        continue;
                    }else{
                        /* Other errors, just close connection ugly */
                        ZERR("Error<%d>: %s, just close connection<%d> ugly.", errno, strerror(errno), fd);
                        if(EBADF != errno){
                            del_fds[del_size++] = fd;
                        }
                        break;
                    }
                }
#else
                zqueue_push(emq->emq, events[i].data.ptr);
#endif
            }else if(events[idx].events & EPOLLOUT){
                /* socket out buffer empty or change notify */
#if ZECHO_TEST
                ZERRC(ZNOT_SUPPORT);
#else
                zqueue_push(emq->emq, events[i].data.ptr);
#endif
            }else{
                ZWAR("not support events<%d>", events[idx].events);
            }
        }/* for(idx)*/

        /* clear */
        for(idx = 0; idx < del_size; ++idx){
            ev.events = EPOLLOUT | EPOLLET;
            ev.data.fd = del_fds[idx];
            zepoll_ctl(epfd, EPOLL_CTL_DEL, del_fds[idx], &ev);
            zsockclose(del_fds[idx]);
        }
    }/* for(;;) */
    ZMSG("epoll proc exit now.");
    return (zthr_ret_t)0;
}
#endif // if 1

zerr_t zemq_run(zemq_t *emq, zvalue_t *hint){
    zerr_t ret = pipe(emq->pipe);
    if(0 == ret){
        ret = zthread_create(&emq->thr, zproc_emq, (zvalue_t)emq);
    }
    return ret;
}

static zerr_t zemq_release_conn(ZOP_ARG){
    zpair_t **pair = (zpair_t**)in;
    zsockclose((*pair)->key.i);
    free(*pair);
    return ZOK;
}

zerr_t zemq_stop(zemq_t *emq, zvalue_t *hint){
    //zthread_cancel(&emq->thr);
    int exit = 1;
    if(-1 == write(emq->pipe[1], &exit, sizeof(exit))){
        ZERRC(errno);
    } /* control exit epoll thread */
    zthread_join(&emq->thr);
    /* Release thread resource */
    zmap_foreach(emq->connections, zemq_release_conn, hint);
    emq->connections->size = 0; /* clear map */
    zsockclose(emq->pipe[0]);
    zsockclose(emq->pipe[1]);
    return ZEOK;
}

#endif
