#ifndef _ZTST_EPOLL_H_
#define _ZTST_EPOLL_H_

#include <zit/net/epoll.h>

/*
 * argv:
 * epoll echo_svr_cli <ehco_number> <client_threads> <conn_per_thr>
 */
#define ZECHO_PORT 9900
static int g_echo_number;
static int g_conn_per_thr;

static zthr_ret_t ZCALL zproc_echo_cli(zvalue_t param){
    /* One connection first.*/
    int i;
    int j;
    //zsock_t fd;
    ztick_t tick;
    zsock_t *fds;
    int nfd = 0;
    int idx = 0;
    /* ... connect to server */
    zsleepsec(1);
    fds = (zsock_t*)calloc(g_conn_per_thr, sizeof(zsock_t));
    for(i=0; i<g_conn_per_thr; ++i){
        fds[i] = zsocket(AF_INET, SOCK_STREAM, 0);
        if(ZINVALID_SOCKET == fds[i]){
            break;
        }else{
            zconnectx(fds[i], "127.0.0.1", ZECHO_PORT, 0, -1);
            zsock_nonblock(fds[i], 0); // set block socket;
            zprint("fds[%d]<%d>\n", i, fds[i]);
            nfd++;
        }
    }

    zsleepsec(1);

    tick = ztick();
    for(i=0; i<g_echo_number; ++i){
    //while(1){
    //    ++i;
        for(idx = 0; idx < nfd; ++idx){
            //zprint("sock<%d> begin send<num: %d>\n", fds[idx], i);
            send(fds[idx], &i, sizeof(int), 0);
            //zprint("sock<%d> begin recv...\n", fds[idx]);
            recv(fds[idx], &j, sizeof(int), 0);
            //zprint("sock<%d> receive<num:%d>\n", fds[idx], j);
            if(i != j){
                zprint("send<%d> != recv<%d>\n", i, j);
            }
        }
    }
    ztock(tick, NULL, NULL);
    for(idx = 0; idx < nfd; ++idx){
        ZDBG("echo_cli close fds[%d]<fd:%d>", idx, fds[idx]);
        zsockclose(fds[idx]);
    }

    // second time connect and send
    nfd = 0;
    for(i=0; i<g_conn_per_thr; ++i){
        fds[i] = zsocket(AF_INET, SOCK_STREAM, 0);
        if(ZINVALID_SOCKET == fds[i]){
            break;
        }else{
            zconnectx(fds[i], "127.0.0.1", ZECHO_PORT, 0, -1);
            zsock_nonblock(fds[i], 0); // set block socket;
            zprint("fds[%d]<%d>\n", i, fds[i]);
            nfd++;
        }
    }

    tick = ztick();
    for(i=0; i<g_echo_number; ++i){
    //while(1){
    //    ++i;
        for(idx = 0; idx < nfd; ++idx){
            //zprint("sock<%d> begin send<num: %d>\n", fds[idx], i);
            send(fds[idx], &i, sizeof(int), 0);
            //zprint("sock<%d> begin recv...\n", fds[idx]);
            recv(fds[idx], &j, sizeof(int), 0);
            //zprint("sock<%d> receive<num:%d>\n", fds[idx], j);
            if(i != j){
                zprint("send<%d> != recv<%d>\n", i, j);
            }
        }
    }
    ztock(tick, NULL, NULL);
    for(idx = 0; idx < nfd; ++idx){
        ZDBG("echo_cli close fds[%d]<fd:%d>", idx, fds[idx]);
        zsockclose(fds[idx]);
    }

    return (zthr_ret_t)0;
}

static zthr_ret_t ZCALL zproc_heart_beat_cli(zvalue_t param){
    /* One connection first.*/
    int i;
    int j;
    zsock_t *fds;
    int nfd = 0;
    int idx = 0;
    char buf[512];

    zsleepsec(1);
    fds = (zsock_t*)calloc(g_conn_per_thr, sizeof(zsock_t));
    for(i=0; i<g_conn_per_thr; ++i){
        fds[i] = zsocket(AF_INET, SOCK_STREAM, 0);
        if(ZINVALID_SOCKET == fds[i]){
            break;
        }else{
            zconnectx(fds[i], "127.0.0.1", ZECHO_PORT, 0, -1);
            zsock_nonblock(fds[i], 0); // set block socket;
            zprint("fds[%d]<%d>\n", i, fds[i]);
            nfd++;
            zsleepsec(2);
        }
    }

    for(i=0; i<g_echo_number; ++i){
        for(idx = 0; idx < nfd; ++idx){
            //zprint("sock<%d> begin send<num: %d>\n", fds[idx], i);
            send(fds[idx], &i, sizeof(int), 0);
            //zprint("sock<%d> begin recv...\n", fds[idx]);
            j = recv(fds[idx], buf, 512, 0);
            ZDBG("sock<%d> receive<bytes:%d>", fds[idx], j);
            zsleepms(200);
        }
    }
    ZDBG("wait normal heart beat now...");
    for(i=0; i<3; ++i){
        // rounds
        for(idx = 0; idx < nfd; ++idx){
            j = recv(fds[idx], buf, 512, 0);
            ZDBG("sock<%d> receive heart beat<bytes:%d>", fds[idx], j);
            send(fds[idx], buf, j, 0);
        }
    }
#if 1
    ZDBG("just receive and not reply, simulate off line....");
    for(i=0; i<1024; ++i){
        for(idx = 0; idx < nfd; ++idx){
            j = recv(fds[idx], buf, 512, 0);
            ZDBG("sock<%d> receive heart beat<bytes:%d> and not reply!", fds[idx], j);
            if(j == 0){
                int down = 1;
                ZDBG("Server close connection<fd:%d>", fds[idx]);
                zsockclose(fds[idx]);
                fds[idx] = ZINVALID_SOCKET;
                for(int ii = 0; ii<nfd; ++ii){
                    /* scan fds, check all fds is invalid */
                    if(fds[ii] != ZINVALID_SOCKET){
                        down = 0;
                        break;
                    }
                }
                if(down){
                    return (zthr_ret_t)0;
                }
            }
        }
    }
#else
    for(idx = 0; idx < nfd; ++idx){
        ZDBG("echo_cli close fds[%d]<fd:%d>", idx, fds[idx]);
        zsockclose(fds[idx]);
        zsleepsec(2);
    }
#endif
    return (zthr_ret_t)0;
}

#define TEMQ_BUF_SIZE 8192 /* 8KB */
#define TEPOLL_BASE 0
#if !TEPOLL_BASE
static zerr_t echo_work(ZOP_ARG){
    zssn_t *ssn = (zssn_t*)hint;
    zemq_t *emq = (zemq_t*)in;
    int32_t readed = 0;
    int32_t readok = 0;
    zerr_t ret;
    //zprint("tepoll.echo_work.ssn<fd:%d>\n", ssn->fd);
    for(;;){ /* May be continue by ENTRY */
        if(0 < (readed = recv(ssn->fd, ssn->packet_buf, ZSSN_PACKET_SIZE, 0))){
            //zprint("receive %d bytes\n", readed);
            ret = zemq_send(emq, ssn, ssn->packet_buf, readed);
            //zprint("send %d bytes data:%d\n", ret, *(int32_t*)ssn->packet_buf);
            readok = 1;
            ZERRCX(ret);
        }else if(0 == readed){
            /* Connection gracefully closed by peer. */
            ZMSG("Connection<%d> gracefully closed by peer.", ssn->fd);
            ret = ZEOF;
            break;
        }else if(errno == EAGAIN ){
            /* In buffer is empty */
            ret = readok ? ZAGAIN : ZEOF;
            time(&ssn->last_operate);
            break;
        }else if(errno == EINTR){
            /* Try again. */
            continue;
        }else{
            /* Other errors, just close connection ugly */
            ZERR("Error<readed:%d, errno:%d>: %s, just close connection<%d> ugly.", readed, errno, strerror(errno), ssn->fd);
            ret = ZEOF;
            break;
        }
    }
    return ret;
}

static zerr_t zemq_hb_timeout(ZOP_ARG){
    zssn_t *ssn = *(zssn_t**)in;
    time_t now = *(time_t*)hint;
    if(now >= ssn->heart_beat_stamp){
        return ZOK;
    }
    return ZFAIL;
}

static zerr_t zemq_ssn_timeout(ZOP_ARG){
    time_t now = time(NULL);
    zemq_t *emq = (zemq_t*)hint;
    zssn_t *ssn = (zssn_t*)in;

    if((now - ssn->last_operate) > emq->ssn_timeout){
        zssn_ctl_t ssn_ctl = {0};
        /* session time out and close it. */
        ZWAR("ssn<fd:%d> time out", ssn->fd);
        zemq_ssn_ctl(emq, &ssn_ctl, ssn, EPOLL_CTL_DEL, EPOLLOUT);
    }
    return ZOK;
}
static zerr_t do_idle(ZOP_ARG){
    zemq_t *emq = (zemq_t*)in;
    time_t now = time(NULL);
    /*
     * Control do idle interval
     */
    if((now - emq->last_syn_idle) < 2){
        return ZOK;
    }
    /*
     * Do idle
     */
    if(zatm_bcas(&emq->idle_status, 0, 1)){
        /* do sync idle */
        if(emq->heart_beat){
            zbtnode_t *head = NULL;
            zbtnode_t *nod = NULL;
            zbtnode_t *prev_nod = NULL; /* for last_operate > (now - heart_beat) */
            zssn_t *ssn = NULL;
            int hb_data = 0;
            int sended = 0;
            int needsend = (int)sizeof(int);
            /* session heart beats
             */
            if(ZOK == zrbtree_pop_front(emq->heart_beats, &head, zemq_hb_timeout, &now)){
                nod = head;
                while(nod){
                    ssn = *(zssn_t**)nod->data;
                    if(ssn->last_operate > (now - emq->heart_beat)){
                        /* not need heart beat, just insert again */
                        ssn->heart_beat_stamp = ssn->last_operate + emq->heart_beat;
                        if(prev_nod){
                            /* not head */
                            prev_nod->next = nod->next;
                        }else{
                            /* reset head */
                            head = nod->next;
                        }
                        nod->parent = nod->left = nod->right = nod->next = NULL;
                        zrbtree_insert(emq->heart_beats, nod);
                        ZDBG("ssn<fd:%d> not need heart beat, just insert again.", ssn->fd);
                    }else{
                        /* heart beat and insert again */
                        sended = send(ssn->fd, &hb_data, needsend, 0);
                        if(sended == needsend){
                            /* set next heart beat timestamp */
                            ssn->heart_beat_stamp = now + emq->heart_beat;
                            prev_nod = nod;
                            ZDBG("ssn<fd:%d> send heart beat OK.", ssn->fd);
                        }else if(-1 == sended && EAGAIN != errno && EINTR != errno){
                            /* May be socket error, recycle node */
                            prev_nod ? (prev_nod->next = nod->next) : (head = nod->next);
                            zrbtree_push_node(emq->heart_beats, &nod);
                            ZDBG("ssn<fd:%d> send heart beat FAIL. erase it ugly.", ssn->fd);
                            ZERRC(errno);
                        }
                    }
                    nod = nod->next;
                }
                /* insert heart beat nodes again */
                if(head){
                    head->parent = head->left = head->right = NULL;
                    zrbtree_insert(emq->heart_beats, head);
                }
            }/* if heart beat */

            if(emq->ssn_timeout){
                /* Check session timeout.
                 * If timeout then close session.
                 */
                zspin_lock(&emq->sessions->spin);
                zrbtree_foreach(emq->sessions->root, 0, zemq_ssn_timeout, (zvalue_t)emq);
                zspin_unlock(&emq->sessions->spin);
            }/* if ssn_timeout */
        }
        time(&emq->last_syn_idle);
        zatm_bcas(&emq->idle_status, 1, 0);
    }else{
        /* do other idle work */
    }
    return ZOK;
}
#endif
static void tepoll(int argc, char **argv){
    if(argc == 6 && 0 == strcmp("echo_svr_cli", argv[2])){
        zemq_t emq = {0};
        int client_threads = atoi(argv[4]);
        g_echo_number = atoi(argv[3]);
        g_conn_per_thr = atoi(argv[5]);
        zthr_id_t *clis = (zthr_id_t*)calloc(client_threads, sizeof(zthr_id_t));
        /* init and run emq*/
        zemq_init(&emq, NULL, ZECHO_PORT, NULL, TEMQ_BUF_SIZE);
        /* config emq */
#if !TEPOLL_BASE /* 0-base 1-do_work*/
        emq.do_work = echo_work;
        emq.worker_num = 2;
#endif
        zemq_run(&emq, NULL); /* run server */
        /* Run all echo clients */
        for(int i=0; i<client_threads; i++){
            zthread_create(&clis[i], zproc_echo_cli, NULL);
        }

        while(--client_threads >= 0){
            /* Wait all client work down */
            zthread_join(&clis[client_threads]);
        }
        zsleepsec(2);
        zemq_stop(&emq, NULL);
        zemq_fini(&emq, NULL);
    }else if(0 == strcmp("echo_svr", argv[2])){
        zemq_t emq = {0};
        /* init and run emq*/
        zemq_init(&emq, NULL, ZECHO_PORT, NULL, TEMQ_BUF_SIZE);
        /* config emq */
#if !TEPOLL_BASE /* 0-base 1-do_work*/
        emq.do_work = echo_work;
        emq.worker_num = 2;
        emq.idle_work = do_idle;
        emq.heart_beat = 5;
        emq.ssn_timeout = 30;
#endif
        zemq_run(&emq, NULL); /* run server */
        zsleepsec(3600);
        zemq_stop(&emq, NULL);
        zemq_fini(&emq, NULL);
    }else if(0 == strcmp("echo_cli", argv[2])){
        int client_threads = atoi(argv[4]);
        g_echo_number = atoi(argv[3]);
        g_conn_per_thr = atoi(argv[5]);
        zthr_id_t *clis = (zthr_id_t*)calloc(client_threads, sizeof(zthr_id_t));
                /* Run all echo clients */
        for(int i=0; i<client_threads; i++){
            zthread_create(&clis[i], zproc_echo_cli, NULL);
        }

        while(--client_threads >= 0){
            /* Wait all client work down */
            zthread_join(&clis[client_threads]);
        }
        ZDBG("client down.");
    }else if(0 == strcmp("heart_beat_cli", argv[2])){
        int client_threads = atoi(argv[4]);
        g_echo_number = atoi(argv[3]);
        g_conn_per_thr = atoi(argv[5]);
        zthr_id_t *clis = (zthr_id_t*)calloc(client_threads, sizeof(zthr_id_t));
                /* Run all echo clients */
        for(int i=0; i<client_threads; i++){
            zthread_create(&clis[i], zproc_heart_beat_cli, NULL);
        }

        while(--client_threads >= 0){
            /* Wait all client work down */
            zthread_join(&clis[client_threads]);
        }
        ZDBG("client down.");
    }else{
        ZERRC(ZNOT_SUPPORT);
    }
}
#endif
