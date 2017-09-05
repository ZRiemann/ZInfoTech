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
            nfd++;
        }
    }

    zsleepsec(1);

    tick = ztick();
    for(i=0; i<g_echo_number; ++i){
    //while(1){
    //    ++i;
        for(idx = 0; idx < nfd; ++idx){
            //ZDBG("sock<%d> begin send<num: %d>", fds[idx], i);
            send(fds[idx], &i, sizeof(int), 0);
            //ZDBG("sock<%d> begin recv", fds[idx], i);
            recv(fds[idx], &j, sizeof(int), 0);
            //ZDBG("%d ", j);
        }
    }
    ztock(tick, NULL, NULL);
    for(idx = 0; idx < nfd; ++idx){
        zsockclose(fds[idx]);
    }
    return (zthr_ret_t)0;
}

static void tepoll(int argc, char **argv){
    if(argc == 6 && 0 == strcmp("echo_svr_cli", argv[2])){
        zemq_t emq;
        int client_threads = atoi(argv[4]);
        g_echo_number = atoi(argv[3]);
        g_conn_per_thr = atoi(argv[5]);
        zthr_id_t *clis = (zthr_id_t*)calloc(client_threads, sizeof(zthr_id_t));
        zemq_init(&emq, NULL, ZECHO_PORT, NULL);
        zemq_run(&emq, NULL); /* run server */
        /* Run all echo clients */
        for(int i=0; i<client_threads; i++){
            zthread_create(&clis[i], zproc_echo_cli, NULL);
        }

        while(--client_threads >= 0){
            /* Wait all client work down */
            zthread_join(&clis[client_threads]);
        }
        zemq_stop(&emq, NULL);
        zemq_fini(&emq, NULL);
    }else{
        ZERRC(ZPARAM_INVALID);
    }
}
#endif
