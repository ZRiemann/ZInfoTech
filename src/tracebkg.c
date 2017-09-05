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
#include <zit/utility/tracebkg.h>
#include <zit/utility/tracelog.h>
#include <stdio.h>
#include <zit/base/trace.h>
#include <zit/container/queue.h>
#include <zit/thread/spin.h>
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <stdlib.h>
#include <string.h>


//#define MAX_TQ_SIZE 4096
static ztrace zg_traceque;
static zcontainer_t in;
static zspinlock_t in_spin;
static zcontainer_t out;
static zcontainer_t tq_buf;
static zspinlock_t tq_spin;
static int intlen;
static int voidlen;
static zthr_id_t thr_id;
static zsem_t thr_sem;

static zthr_ret_t ZCALL zproc_trace(void*);

int ztrace_bkgbuf(char **buf, int len){
    zprint("get buf begin lock tq_spin...\n");
    zspin_lock(&tq_spin);
    zprint("get buf lock tq_spin ok\n");
    if(ZOK != zque1_pop(tq_buf, (zvalue_t*)buf)){
        *buf = (char*)malloc(len);
    }
    zspin_unlock(&tq_spin);

    zprint("get buf unlock tq_spin\n");
    return *buf ? ZOK : ZMEM_INSUFFICIENT;
}

static int ztq_recyclebuf(char *buf){
    zprint("rec buf begin lock tq_spin...\n");
    zspin_lock(&tq_spin);
    zprint("rec buf end lock tq_spin ok\n");
    if( zque1_size(tq_buf) > 500 ){
        free(buf);
    }else{
        zque1_push(tq_buf, buf);
    }
    zspin_unlock(&tq_spin);
    zprint("rec buf unlock tq_spin\n");
    return ZOK;
}

static zerr_t free_buf(ZOP_ARG){
    free(in);
    return ZOK;
}

int ztrace_bkgctl(ztrace cb){
    int ret;
    int chunk_size = 960;
    ret = ZOK;
    intlen = sizeof(int);
    voidlen = sizeof(void*);
    zg_traceque = cb;

    zspin_init(&in_spin);
    zspin_init(&tq_spin);
    zque1_create(&in, &chunk_size);
    zque1_create(&out, &chunk_size);
    zque1_create(&tq_buf, &chunk_size);
    zsem_init(&thr_sem, 0);
    zthread_create(&thr_id, zproc_trace, NULL);
    return ret;
}

int ztrace_bkgend(){
    int *plen;
    void **puser;
    char *msg;
    char *buf;
    if(!intlen){
        return ZOK; // not call bkgclt(), just return;
    }
    buf = NULL;
    zsem_post(&thr_sem);
    zthread_join(&thr_id);
    zsem_fini(&thr_sem);
    // dump out...
    while(ZOK == zque1_pop(out, (zvalue_t*)&buf)){
        msg = buf;
        msg += (strlen(msg)+1);
        plen = (int*)msg;
        msg += intlen;
        puser = (void**)msg;

        if(zg_traceque){
            zg_traceque(*plen, *puser, buf);
        }
        ztq_recyclebuf(buf);
    }

    while(ZOK == zque1_pop(in, (zvalue_t*)&buf)){
        msg = buf;
        msg += (strlen(msg)+1);
        plen = (int*)msg;
        msg += intlen;
        puser = (void**)msg;

        if(zg_traceque){
            zg_traceque(*plen, *puser, buf);
        }
        ztq_recyclebuf(buf);
    }

    // free buffer
    zque1_destroy(in);
    zque1_destroy(out);
    zque1_foreach(tq_buf, free_buf, NULL);
    zque1_destroy(tq_buf);
    zspin_fini(&tq_spin);
    zspin_fini(&in_spin);
    return ZOK;
}

int ztrace_0cpy_bkg(int len, void *user, const char* msg){
    int *plen;
    void **puser;
    char *buf;

    buf = (char*)msg;
    buf[len] = 0;
    plen = (int*)(buf+len+1);
    puser = (void**)(buf+len+1+intlen);
    *plen = len;
    *puser = user;

    zspin_lock(&in_spin);
    zque1_push(in, buf);
    zspin_unlock(&in_spin);
    return ZOK;
}
int ztrace_bkg(int len, void *user, const char* msg){
    char *buf;
    int *plen;
    void **puser;
    zprint("in: %s\n", msg);
    if(ZOK == ztrace_bkgbuf(&buf, ZTRACE_BUF_SIZE)){
        zprint("in get bug ok\n");
        memcpy(buf, msg, len);
        buf[len] = 0;
        plen = (int*)(buf+len+1);
        puser = (void**)(buf+len+1+intlen);
        *plen = len;
        *puser = user;
        //zprint("++ len:%d user:%p msg<%p>:%s", *plen, *puser, buf, buf);
        zspin_lock(&in_spin);
        zprint("bkg push in\n");
        zque1_push(in, buf);
        zprint("bkg unlock in_spin");
        zspin_unlock(&in_spin);
    }
    return ZOK;
}

zthr_ret_t ZCALL zproc_trace(void* param){
    char *buf;
    char *msg;
    int *plen;
    void **puser;
    msg = NULL;
    buf = NULL;
    while( ZETIMEOUT == zsem_wait(&thr_sem, 100)){
        while(ZOK == zque1_pop(out, (zvalue_t*)&buf)){
            msg = buf;
            msg += (strlen(msg)+1);
            plen = (int*)msg;
            msg += intlen;
            puser = (void**)msg;
            //zprint("-- len:%d user:%p msg<%p>:%s\n", *plen, *puser, buf, buf);
            if(zg_traceque){
                zg_traceque(*plen, *puser, buf);
            }
            ztq_recyclebuf(buf);
        }

        if(zque1_size(in)){
            zprint("begin lock in_spin...\n");
            zspin_lock(&in_spin);
            zprint("begin swap...\n");
            zque1_swap(&in, &out); // swap in/
            zprint("begin end swap\n");
            zspin_unlock(&in_spin);
        }else{
            zprint("in queue is empty\n");
        }
        // else in and out all empty(may be call use timer)

        if(msg){
            // fflush to logfile
            ztrace_logflush();
            msg = NULL;
        }
    }
    return 0;
}
