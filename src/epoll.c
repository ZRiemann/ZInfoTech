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

static zerr_t zemq_hb_cmp_stamp(ZOP_ARG){
    zssn_t *refer = *(zssn_t **)in;
    zssn_t *node = *(zssn_t**)hint;
    zerr_t ret = ZEQUAL;
    if(refer->heart_beat_stamp > node->heart_beat_stamp){
        ret = ZGREAT;
    }else if(refer->heart_beat_stamp < node->heart_beat_stamp){
        ret = ZLITTLE;
    }
    return ret;
}

static zerr_t zemq_hb_cmp_fd(ZOP_ARG){
    zssn_t *refer = *(zssn_t **)in;
    zssn_t *node = *(zssn_t**)hint;
    zerr_t ret = ZEQUAL;
    if(refer->fd > node->fd){
        ret = ZGREAT;
    }else if(refer->fd < node->fd){
        ret = ZLITTLE;
    }
    return ret;
}

zerr_t zemq_init(zemq_t *emq, const char *addr, int32_t port, zvalue_t hint,
                 int32_t alloc_buf_size){
    if(!emq || (port < 0 && port > 65535)){
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
    zrbtree_create(&emq->sessions,
                   (int32_t)sizeof(zssn_t),
                   256, NULL, 0);
    zrbtree_create(&emq->heart_beats,
                   (int32_t)sizeof(zvalue_t),
                   256, zemq_hb_cmp_stamp, 1);
    zalloc_create(&emq->buf_alloc, alloc_buf_size, 256, 256*1024*1024);
    zsem_init(&emq->sem_emq, 0);
    zspin_init(&emq->spin_in);
    zspin_init(&emq->spin_out);
    zspin_init(&emq->spin_cmd);
#ifdef ZTRACE_EPOLL
    ZERRC(ZOK);
#endif
    return ZOK;
}
zerr_t zemq_fini(zemq_t *emq, zvalue_t *hint){
    /* Destroy all resource... */
    zqueue_destroy(emq->emq);
    zrbtree_destroy(emq->sessions);
    zrbtree_destroy(emq->heart_beats);
    zalloc_destroy(emq->buf_alloc);
    zsem_fini(&emq->sem_emq);
    zspin_fini(&emq->spin_in);
    zspin_fini(&emq->spin_out);
    zspin_fini(&emq->spin_cmd);
#ifdef ZTRACE_EPOLL
    ZERRC(ZOK);
#endif
    return ZOK;
}

#define ZMAX_EVENTS 16
#define ZECHO_TEST 1
#define ZECHO_BUF_SIZE 256

/* erase session in heart beat tree */
zerr_t zemq_hb_format(ZOP_ARG){
    static time_t base = 0;
    zssn_t *ssn = *(zssn_t**)in;
    char *buf = (char*)hint;
    if(0 == base){
        time(&base);
    }
    sprintf(buf, "(%d,%d)", ssn->fd, (int)(ssn->heart_beat_stamp - base));
    return 0;
}
zinline void zemq_hb_print(zemq_t *emq){
    zrbtree_printx(emq->heart_beats, zemq_hb_format);
}
zinline zerr_t zemq_hb_get(zemq_t *emq, zssn_t **ssn, zbtnode_t **nod){
    int ret = zrbtree_get_node(emq->heart_beats, nod);
    if(ZOK == ret){
        memcpy((*nod)->data, ssn, sizeof(zvalue_t));
    }else{
        ZERRC(ret);
    }
    return ret;
}
zinline zerr_t zemq_hb_push(zemq_t *emq, zbtnode_t **nod){
    return zrbtree_recycle_node(emq->heart_beats, nod);
}

zinline zerr_t zemq_hb_insert(zemq_t *emq, zbtnode_t *nod){
    return zrbtree_insert(emq->heart_beats, nod);
}

zinline void zemq_hb_erase(zemq_t *emq, zssn_t *ssn){
    if(emq->heart_beat){
        zbtnode_t *cmp_nod = NULL;
        int ret = ZEOK;
        if(ZEOK == (ret = zemq_hb_get(emq, &ssn, &cmp_nod))){
            ret = zrbtree_erasex(emq->heart_beats, cmp_nod, zemq_hb_cmp_fd);
            //zemq_hb_print(emq);
        }
        if(cmp_nod){
            zemq_hb_push(emq, &cmp_nod);
        }
        ZERRCX(ret);
    }
}

zinline zerr_t zemq_insert_ssn(zemq_t *emq, zssn_t *ssn){
    zbtnode_t *nod = (zbtnode_t*)zmember2st(zbtnode_t, data, ssn);
    //ZDBG("Insert node<ptr:%p key:%d ssn->fd:%d>", nod, zrbn_key(nod), ssn->fd);
    time(&ssn->last_operate);
    if(emq->heart_beat){
        zbtnode_t *nod = NULL;
        ssn->heart_beat_stamp = ssn->last_operate + emq->heart_beat;
        if(ZOK == zemq_hb_get(emq, &ssn, &nod)){
            zemq_hb_insert(emq, nod);
            //zemq_hb_print(emq);
        }else{
            ZERRC(ZFAIL);
        }
    }
    return zrbtree_insert(emq->sessions, nod);
}
zinline zerr_t zemq_erase_ssn(zemq_t *emq, zsock_t fd){
    zbtnode_t nod = {0};
    *(int32_t*)(nod.data) = fd;
    //ZDBG("erase ssn<fd:%d>", fd);
    return zrbtree_erase(emq->sessions, &nod);
}

zinline void zemq_foreach_ssn(zemq_t *emq, zoperate op, zvalue_t hint){
    zrbtree_foreach(emq->sessions->root, 0, op, hint);
}

zerr_t zemq_get_buf(zemq_t *emq, zssn_t *ssn, int is_read){
    zvalue_t *buf = (zvalue_t*)(is_read ? &ssn->recv_buf : &ssn->send_buf);
    if(!*buf){
        //is_read ? (ssn->recv_buf_len = 0) : (ssn->send_buf_len = 0);
        return zalloc_pop(emq->buf_alloc, buf);
    }
    return ZEOK;
}

zerr_t zemq_push_buf(zemq_t *emq, zssn_t *ssn, int is_read){
    zvalue_t buf = NULL;
    if(is_read){
        buf = ssn->recv_buf;
        ssn->recv_buf_len = 0;
        ssn->recv_buf = NULL;
    }else{
        buf = ssn->send_buf;
        ssn->send_buf_len = 0;
        ssn->send_buf = NULL;
    }
    if(buf){
        return zalloc_push(emq->buf_alloc, &buf);
    }
    return ZEOK;
}

zinline zerr_t zemq_init_ssn(zemq_t *emq, zssn_t *ssn){
    ssn->fd = ZINVALID_SOCKET;
    zspin_init(&ssn->spin_send);
    zspin_init(&ssn->spin_status);
    return ZEOK;
}
zinline zerr_t zemq_fini_ssn(zemq_t *emq, zssn_t *ssn){
    if(ssn->fd != ZINVALID_SOCKET){
        struct epoll_event ev = {0};
        ev.events = EPOLLOUT | EPOLLET;
        ev.data.fd = ssn->fd;
        zepoll_ctl(emq->epfd, EPOLL_CTL_DEL, ssn->fd, &ev);
        //ZDBG("CLOSE ssn<fd:%d>", ssn->fd);
        zsockclose(ssn->fd);
    }
    zemq_push_buf(emq, ssn, 0);
    zemq_push_buf(emq, ssn, 1);
    zspin_fini(&ssn->spin_send);
    zspin_fini(&ssn->spin_status);
    return ZEOK;
}


zinline zssn_t *zemq_get_ssn(zemq_t *emq){
    zbtnode_t *nod = NULL;
    zssn_t *ssn = NULL;
    if(ZOK == zrbtree_get_node(emq->sessions, &nod)){
        ssn = (zssn_t*)nod->data;
        zemq_init_ssn(emq, ssn);
    }
    return ssn;
}

zinline zerr_t zemq_ctl_ssn(zemq_t *emq, zssn_t *ssn, int ctl, uint32_t events){
    struct epoll_event ev = {0};
    ev.events = events;
    ev.data.fd = ssn->fd;
    return zepoll_ctl(emq->epfd, ctl, ssn->fd, &ev);
}
/**
 * @brief Recycle session memory to emq queue
 */
zinline zerr_t zemq_push_ssn(zemq_t *emq, zssn_t *ssn){
    /* erase heart beat map */
    zemq_hb_erase(emq, ssn);
    /* erase session map */

    ZDBG("total ssns<%d> del<fd:%d>", emq->total_ssns, ssn->fd);
    if(emq->total_ssns < 10){
        zrbtree_print(emq->sessions);
    }
    zemq_fini_ssn(emq, ssn);
    if(ZOK == zemq_erase_ssn(emq, ssn->fd)){
        --emq->total_ssns;
        return ZOK;
    }
    ZWAR("erase ssn<fd:%d> fail", ssn->fd);
    return ZFAIL;
}

/**
 * @breif Try Send <buf> by <len> bytes.
 * @param sock [in] socket file descripter
 * @param buf [in] buffer to send
 * @param len [in|out] in: bytes to send
 *                     out: bytes actually sended
 * @return ZEOK/Z*
 */
zinline zerr_t zemq_sendn(zsock_t sock, const char *buf, int *len){
    zerr_t ret = ZEOK;
    int sended = 0;
    int size = *len;
    while(sended != size){
        /* dump */
        ret = send(sock, buf+sended, *len, 0);
        if(ret >= 0){
            sended += ret;
            ret = ZEOK;
        }else{
            ret = errno;
            if(EAGAIN == ret){
                ret = ZAGAIN;
                break; /** use poll out */
            }
            if(EWOULDBLOCK == ret || EINTR == ret){
                ZERRC(ret);
                continue;
            }else{
                ZERRC(ret);
                break;
            }
        }
    }
    if(ZAGAIN == ret){
        *len = sended;
    }
    return ret;
}
/**
 * @brief send or poll out <buf> by len size on session<ssen>,
 *        parallel (single)recv and (single)send
 * @param emq [in] pointer to zemq_t
 * @param ssn [in] pointer to session
 * @param buf [in] pointer to buf
 * @param len [in] <buf> length
 * @return ZOK/Z*
 */
zerr_t zemq_send(zemq_t *emq, zssn_t *ssn, char *buf, int len){
    int ret = ZEOK;
    int sended = 0;
    zspin_lock(&ssn->spin_send);
    /* 1. check <ssn->send_buf>, and try send it */
    if(ssn->send_buf){
        sended = ssn->send_buf_len;
        if(ZEOK == (ret = zemq_sendn(ssn->fd, ssn->send_buf, &sended))){
            zemq_push_buf(emq, ssn, 0);
            zemq_ctl_ssn(emq, ssn, EPOLL_CTL_MOD, EPOLLIN | EPOLLET);
            zprint("zemq_send(ssn<fd: %d> sended buf_len<%d>)", ssn->fd, len);
        }
        if(ret == ZEAGAIN){
            /* 1.1 pull out */
            ssn->send_buf_len -= sended;
            memmove(ssn->send_buf, ssn->send_buf + sended, ssn->send_buf_len);
            if(len + ssn->send_buf_len > emq->buf_size){
                ret = ZEMEM_INSUFFICIENT;
                ZERRC(ret);
            }else if(buf){
                memcpy(ssn->send_buf + ssn->send_buf_len, buf, len);
                ssn->send_buf_len += len;
                //zemq_ctl_ssn(emq, ssn, EPOLL_CTL_MOD, EPOLLIN | EPOLLOUT | EPOLLET);
                zprint("zemq_send(ssn<fd: %d> poll out buf_len<%d>)", ssn->fd, len);
            }
        }
    }
    /* 2. send <buf> or append to <ssn->send_buf>*/
    if(ZEOK == ret && buf){
        /* 2.1 send <buf> directory */
        sended = len;
        if(ZEOK != (ret = zemq_sendn(ssn->fd, buf, &sended))){
            /* 2.2 if not send all, register poll out */
            if(ret == ZEAGAIN){
                zemq_get_buf(emq, ssn, 0);
                memcpy(ssn->send_buf, buf, len - sended);
                ssn->send_buf_len = len - sended;
                zemq_ctl_ssn(emq, ssn, EPOLL_CTL_MOD, EPOLLIN | EPOLLOUT | EPOLLET);
                zprint("zemq_send(ssn<fd: %d> poll out buf_len<%d>)", ssn->fd, len);
            }else{
                ZERRC(ret);
            }
        }
    }
    /* not destroy ssn by send operate 
    if(ret != ZEOK && ret != ZEAGAIN && ret != ZEMEM_INSUFFICIENT){
        zemq_push_ssn(emq, ssn);
    }
    */
    zspin_unlock(&ssn->spin_send);
    return ret;
}

#if 1
zinline zerr_t zbuild_emq(zemq_t *emq, int *epfd, int *listenfd, int *ctlfd){
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
        /* add cmd pipe to epoll by level trigger */
        ev.events = EPOLLIN | EPOLLRDHUP;
        ev.data.ptr = &emq->pipe[0];
        ret = zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, emq->pipe[0], &ev);

        *epfd = emq->epfd;
        *listenfd = emq->listenfd;
        *ctlfd = emq->pipe[0];
    }while(0);

    return ret;
}

zinline void zaccept_session(zemq_t *emq){
    struct epoll_event ev = {0};
    zssn_t *ssn = NULL;
    int len;
    zsock_t fd;
    zsockaddr_in addr;

    for(;;){
        len = sizeof(zsockaddr_in);
        if(0 > (fd = zaccept(emq->listenfd, (ZSA*)&addr, &len))){
            if(errno == ECONNABORTED || errno == EPROTO || errno == EINTR){
                ZDBG("May be more connection, error: %s", strerror(errno));
                continue;
            }else{
                break; /* No more connection */
            }
        }
        if(NULL == (ssn = zemq_get_ssn(emq))){
            zsockclose(fd);
            break;
        }
        ssn->fd = fd;
        ssn->addr = addr;
        zsock_nonblock(ssn->fd, 1);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = ssn;
        if(ZOK != zepoll_ctl(emq->epfd, EPOLL_CTL_ADD, ssn->fd, &ev)){
            zemq_push_ssn(emq, ssn);
        }else{
            ++emq->total_conns;
            ++emq->total_ssns;
            ZDBG("active ssns<%d> add<fd:%d>", emq->total_ssns, ssn->fd);
            zemq_insert_ssn(emq, ssn);
        }
    }
}

zinline zerr_t zepoll_in(zemq_t *emq, zssn_t* ssn){
    zerr_t ret = ZOK;
    zspin_lock(&ssn->spin_status);
    /* 1. increment poll in hits */
    zatm_inc(&ssn->pin_hits);
    //zprint("new poll int hists: %d\n", ssn->pin_hits);
    ssn->events |= EPOLLIN;
    /* 2. check busy or idle status */
    ret = zatm_bcas(&ssn->status,ZSSN_STAT_IDLE, ZSSN_STAT_IDLE);
    zspin_unlock(&ssn->spin_status);
    if(ret){
        /* 3. notify worker threads */
        //zprint("set ssn<fd:%d> ready status, push emq\n", ssn->fd);
        zatm_xchg(&ssn->status, ZSSN_STAT_READY);
        ret = zqueue_lock_push(emq->emq, (zvalue_t)&ssn, &emq->spin_in);
        zsem_post(&emq->sem_emq);
    }else{
        //zprint("ssn<fd:%d> is ready status, not push\n", ssn->fd);
    }
    ZERRCX(ret);
    return ret;
}

zinline zerr_t zepoll_out(zemq_t *emq, zssn_t *ssn){
    int ret = ZOK;
    /* 1. increment poll out hits */
    zatm_inc(&ssn->pout_hits);
    ssn->events |= EPOLLOUT;
    zatm_xchg(&ssn->status, ZSSN_STAT_READY);
    ret = zqueue_lock_push(emq->emq, (zvalue_t)ssn, &emq->spin_in);
    zsem_post(&emq->sem_emq);
    ZERRCX(ret);
    return ret;
}

zerr_t zemq_ssn_ctl(zemq_t *emq, zssn_ctl_t *ctl,
                    zssn_t *ssn, int op, uint32_t events){
    int needwrite;
    int writed;
    char *pc = (char*)ctl;
    ctl->ssn = ssn;
    ctl->op = op;
    ctl->ev.events = events;
    ctl->ev.data.ptr = (zvalue_t)ssn;
    needwrite = sizeof(zssn_ctl_t);
    zspin_lock(&emq->spin_cmd);
    while(needwrite){
        writed = write(emq->pipe[1], pc+(sizeof(zssn_ctl_t) - needwrite), needwrite);
        if(writed > 0){
            needwrite -= writed;
        }else{
            ZERRC(errno);
        }
    }
    zspin_unlock(&emq->spin_cmd);
    return ZOK;
}
static zthr_ret_t ZCALL zproc_emq(void* param){
    zemq_t *emq = (zemq_t*)param;
    struct epoll_event events[ZMAX_EVENTS];
    zsock_t listenfd = ZINVALID_SOCKET;
    zsock_t curfd = ZINVALID_SOCKET;
    zsock_t ctlfd = ZINVALID_SOCKET;
    zsock_t epfd = ZINVALID_SOCKET;
    int32_t idx, nfds;
    int is_run = 1;
    zssn_ctl_t ssn_ctl = {0};
    ssize_t readed;
    size_t needread;
    if(ZOK != zbuild_emq(emq, &epfd, &listenfd, &ctlfd)){
        return 0;
    }
    ZDBG("zemq server running...");
    while(is_run){
        if(-1 == (nfds = zepoll_wait(epfd, events, ZMAX_EVENTS, -1))){
            ZERRC(errno);
            return 0;
        }
        for(idx = 0; idx < nfds; ++idx){
            curfd = *((zsock_t*)(events[idx].data.ptr));
            if(curfd == ctlfd){
                /* exit / delete / mod command in*/
                for(;;){
                    /* read(ctlfd, &ssn_ctl, sizeof(ssn_ctl_t)); */
                    needread = sizeof(zssn_ctl_t);
                    readed = 0;
                    while(needread){
                        readed = read(ctlfd, &ssn_ctl + sizeof(zssn_ctl_t) - needread,
                                      needread);
                        if(readed > 0){
                            needread -= readed;
                        }else if(0 == readed){
                            is_run = 0; /* command fd is closed */
                            ZDBG("zemq receive exit command");
                            break;
                        }else if(EINTR == errno){
                            continue; /* interrupt try again */
                        }else if(EAGAIN == errno){
                            break; /* No more commands */
                        }else{
                            is_run = 0; /* any other error occur just exit ugly. */
                            ZERRC(errno);
                            break;
                        }
                    }
                    if(0 == needread){
                        if(ssn_ctl.ssn == NULL){
                            /* exit command */
                            is_run = 0;
                        }else{
                            if(ssn_ctl.op == EPOLL_CTL_DEL){
                                zemq_push_ssn(emq, ssn_ctl.ssn);
                            }else if(ZOK != zepoll_ctl(emq->epfd, ssn_ctl.op,
                                                       ssn_ctl.ssn->fd, &ssn_ctl.ev)){
                                zemq_push_ssn(emq, ssn_ctl.ssn);
                            }
                        }
                    }else{
                        break;
                    }
                }/* for(;;)*/
            }else if(curfd == listenfd){
                /* accept new session */
                zaccept_session(emq);
            }else if(events[idx].events & EPOLLIN){
                /* in buffer notify */
                zepoll_in(emq, (zssn_t*)events[idx].data.ptr);
            }else if(events[idx].events & EPOLLOUT){
                /* out bffer notify */
                zepoll_out(emq, (zssn_t*)events[idx].data.ptr);
            }else{
                /* some other events */
                ZWAR("not support events<%d>", events[idx].events);
            }
        }/*for idx */
    }/* while is run */
    ZDBG("zemq servre exit now.");
    return NULL;
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
                // set busy
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
                // sleep 100
                // set idle
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

static zthr_ret_t ZCALL zproc_worker(void* param){
    zemq_t *emq = (zemq_t*)param;
    zerr_t ret = ZOK;
    zssn_t **ppssn = NULL;
    zssn_t *ssn = NULL;
    int more_hits = 0;
    int32_t hits_pin = 0;
    char packet_buf[ZSSN_PACKET_SIZE]; /* packet buffer, avoid memory allocate */
    zssn_ctl_t ssn_ctl = {0}; /* session control */
    int idle_cnt = 0;

    ZDBG("emq.zproc_worker running...");
    while(!emq->is_run || *emq->is_run){
        ret = zsem_wait(&emq->sem_emq, 1000);
        if(ZOK == ret){
            while(ZOK == (ret = zqueue_lock_pop(emq->emq, (zvalue_t*)&ppssn, &emq->spin_out))){
                /* loop and pop all work */
                /* handle poll in / out*/
                ssn = *ppssn;
                zprint("zproc_worker.pop.ssn<fd:%d>\n", ssn->fd);
                do{
                    hits_pin = zatm_cas(&ssn->pin_hits, 0, 0);
                    zprint("old poll in hits: %d\n", hits_pin);
                    if(ssn->events & EPOLLIN){
                        /* do poll in by user define */
                        ssn->packet_buf = packet_buf;
                        ret = emq->do_work((zvalue_t)emq, NULL, (zvalue_t)ssn);
                    }
                    if(ssn->events & EPOLLOUT){
                        /* do poll out atomic */
                        zprint("poll out\n");
                        ret = zemq_send(emq, ssn, NULL, 0);
                        ZERRCX(ret);
                    }
                    /* lock session status */
                    zspin_lock(&ssn->spin_status);
                    more_hits = !zatm_bcas(&ssn->pin_hits, hits_pin, hits_pin);
                    if(!more_hits){
                        zprint("set ssn<fd:%d> idle status\n", ssn->fd);
                        ssn->events &= (~EPOLLIN);
                        zatm_xchg(&ssn->status, ZSSN_STAT_IDLE);
                    }else{
                        zprint("ssn<fd:%d> has more hits\n", ssn->fd);
                    }
                    zspin_unlock(&ssn->spin_status);
                    /* unlock session status*/

                    if(ZEOF == ret){
                        /* close session */
                        zemq_ssn_ctl(emq, &ssn_ctl, ssn, EPOLL_CTL_DEL, EPOLLOUT);
                        break;
                    }
                }while(more_hits);

                if(emq->is_run && !*emq->is_run){
                    break; /* fast exit */
                }
            }

        }else if(ZTIMEOUT == ret){
            if(!(++idle_cnt & 0xff)){
                ZDBG("worker<id: %d, is_run: %p, %d> idle...\n", zthread_self(), emq->is_run, emq->is_run ? *emq->is_run : -1);
            }
            if(emq->idle_work){
                emq->idle_work((zvalue_t)emq, NULL, NULL);
                time(&emq->last_idle);
            }
        }else{
            ZERRC(ret);
        }
        /* Force idle work, in busy system */
        if(emq->idle_work){
            time_t now = time(NULL);
            if(now - emq->last_idle > 3){
                emq->idle_work((zvalue_t)emq, NULL, NULL);
                time(&emq->last_idle);
            }
        }
    }
    if(emq->down_work){
        emq->down_work((zvalue_t)emq, NULL, NULL);
    }
    ZDBG("emq.zproc_worker exit now.\n");
    return 0;
}
zerr_t zemq_run(zemq_t *emq, zvalue_t *hint){
    zerr_t ret = pipe(emq->pipe);
    int i = 0;
    if(0 == ret){
        /* create worker threads */
        int actual_workers = 0;
        zsock_nonblock(emq->pipe[0], 1);
        zsock_nonblock(emq->pipe[1], 1);
        if(emq->worker_num > 32){
            emq->worker_num = 32;
        }
        emq->workers = (zthr_id_t*)calloc(emq->worker_num, sizeof(zthr_id_t));
        for(i=0; i<emq->worker_num; ++i){
            if(ZOK == zthread_create(&emq->workers[i], zproc_worker, (zvalue_t)emq)){
                ++actual_workers;
            }else{
                break;
            }
        }
        emq->worker_num = actual_workers;
        /* create epoll thread */
        ret = zthread_create(&emq->thr, zproc_emq, (zvalue_t)emq);
    }
    return ret;
}

static zerr_t zemq_release_conn(ZOP_ARG){
    zbtnode_t *nod = (zbtnode_t*)in;
    zsock_t fd= ((zssn_t*)nod->data)->fd;
    zsockclose(fd);
    zrbtree_recycle_node(((zemq_t*)hint)->sessions, &nod); /*recycle nodes*/
    return ZOK;
}

zerr_t zemq_stop(zemq_t *emq, zvalue_t *hint){
    int exit = 1;
    int i = 0;
    zssn_ctl_t ctl;
    /* control exit poll thread */
    zemq_ssn_ctl(emq, &ctl, NULL, 0, 0);

    zthread_join(&emq->thr);
    /* Release thread resource */
    zemq_foreach_ssn(emq, zemq_release_conn, (zvalue_t)emq);
    emq->sessions->root = NULL; /* clear rbtree */
    zsockclose(emq->pipe[0]);
    zsockclose(emq->pipe[1]);
    /* Stop worker threads */
    if(!emq->is_run){
        exit = 0;
        emq->is_run = &exit;
    }
    for(i=0; i<emq->worker_num; ++i){
        zthread_join(&emq->workers[i]);
    }
    return ZEOK;
}

#endif
