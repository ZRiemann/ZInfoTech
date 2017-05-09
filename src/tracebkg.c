#include "export.h"
#include <zit/utility/tracebkg.h>
#include <stdio.h>
#include <zit/base/trace.h>
#include <zit/base/queue.h>
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <stdlib.h>
#include <string.h>


//#define MAX_TQ_SIZE 4096
static ztrace zg_traceque;
static zcontainer_t in;
static zcontainer_t out;
static zcontainer_t tq_buf;
static int intlen;
static int voidlen;
static zthr_t thr_trace;
static zthr_ret_t ZCALL zproc_trace(void*);

int ztrace_bkgbuf(char **buf, int len){
  if(ZOK == zque_pop(tq_buf, (zvalue_t*)buf)){
    return ZOK;
  }else{
    *buf = (char*)malloc(len);
  }
  return *buf ? ZOK : ZMEM_INSUFFICIENT;
}
static int ztq_recyclebuf(char *buf){
  if( zque_size(tq_buf) > 500 ){
    free(buf);
  }else{
    zque_push(tq_buf, buf);
  }
  return ZOK;
}

static int free_buf(ZOP_ARG){
  free(in);
  return ZOK;
}

int ztrace_bkgctl(ztrace cb){
  int ret;
  ret = ZOK;
  intlen = sizeof(int);
  voidlen = sizeof(void*);
  zg_traceque = cb;

  zque_create(&in);
  zque_create(&out);
  zque_create(&tq_buf);

  zthreadx_create(&thr_trace, zproc_trace);
  return ret;
}

int ztrace_bkgend(){
  int *plen;
  void **puser;
  char *msg;
  char *buf;
  zthreadx_cancel(&thr_trace);
  zthreadx_join(&thr_trace);
  // dump out...
  while(ZOK == zque_pop(out, (zvalue_t*)&buf)){
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

  while(ZOK == zque_pop(in, (zvalue_t*)&buf)){
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
  zque_destroy(in, 0);
  zque_destroy(out, 0);
  zque_destroy(tq_buf, free_buf);
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

  zque_push(in, buf);
  return ZOK;
}
int ztrace_bkg(int len, void *user, const char* msg){
  char *buf;
  int *plen;
  void **puser;
  if(ZOK == ztrace_bkgbuf(&buf, ZTRACE_BUF_SIZE)){
    memcpy(buf, msg, len);
    buf[len] = 0;
    plen = (int*)(buf+len+1);
    puser = (void**)(buf+len+1+intlen);
    *plen = len;
    *puser = user;
    //printf("++ len:%d user:%p msg<%p>:%s", *plen, *puser, buf, buf);
    zque_push(in, buf);
  }
  return ZOK;
}

zthr_ret_t ZCALL zproc_trace(void* param){
  int ret;
  zthr_t* thr;
  char *buf;
  char *msg;
  int *plen;
  void **puser;
  
  ret = ZOK;
  thr = (zthr_t*)param;
  if(ZEOK != zthreadx_procbegin(thr)){
    zthreadx_procend(thr, ret);
    return 0;
  }
  while( ZETIMEOUT == zsem_wait(&(thr->exit), 100)){
    while(ZOK == zque_pop(out, (zvalue_t*)&buf)){
      msg = buf;
      msg += (strlen(msg)+1);
      plen = (int*)msg;
      msg += intlen;
      puser = (void**)msg;
      //printf("-- len:%d user:%p msg<%p>:%s\n", *plen, *puser, buf, buf);
      if(zg_traceque){
	zg_traceque(*plen, *puser, buf);
      }
      ztq_recyclebuf(buf);
    }
    
    if(zque_size(in)){
      zque_swap(&in, &out); // swap in/
    }// else in and out all empty(may be can use timer)
  }
  zthreadx_procend(thr, ret);

  return 0;
}
