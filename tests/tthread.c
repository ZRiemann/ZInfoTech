#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/thread/thread.h>
#include <zit/base/time.h>
#include <zit/base/error.h>
#include <stdio.h>
#include "tthread.h"

#ifdef ZSYS_WINDOWS
#pragma warning(disable:4996)
#endif
void ztst_thread(){
  //ztst_semaphore();
  //ztst_mutex();
  //ztst_thrctl();
  ztst_thrxctl();
}

void ztst_mutex(){
  zmutex_t mtx;
  zmutex_init(&mtx);
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  zmutex_uninit(&mtx);

  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  
}
void ztst_semaphore(){
  zsem_t sem;
  //zsem_t sem1;
  //ZMSG("test normal...");
  zsem_init(&sem,0);
  zsem_post(&sem);
  zsem_wait(&sem, ZINFINITE);
  zsem_wait(&sem, 1000);
  zsem_uninit(&sem);
  
  //ZMSG("test sem destroyed...");
  //zsem_post(&sem1);
  //zsem_init(&sem1, 0);
  //zsem_wait(&sem1,100);
  //zsem_wait(&sem1,ZINFINITE);
}

zthr_ret_t ZAPI zproc_thr1(void* param){
  zthr_attr_t* attr = (zthr_attr_t*)param;
  int i = 0;
  
  ZDBG("thread[%s] beging...", attr->name);
  for(;i<10;i++){
    ZDBG("thread %s run %d...", attr->name, i);
    zsleepms(500);
  }
  ZDBG("thread[%s] end.", attr->name);

  return (zthr_ret_t)ZEOK;
}

zthr_ret_t ZAPI zproc_thr2(void* param){
  int ret = ZEOK;
  zthr_attr_t* attr = (zthr_attr_t*)param;
  void* user_param = attr->param; // for user parameter
  //ZDBG("thread[%s] running...");
  if(ZEOK != zthreadx_procbegin(attr)){
    zthreadx_procend(attr, ret);
    return (zthr_ret_t)ZEFAIL;
  }
  while( ZETIMEOUT == zsem_wait(&(attr->exit), 500)){
    // loop working...
    ZDBG("%s working...", attr->name);
  }
  zthreadx_procend(attr, ret);
  //ZDBG("thread[%s] exit now.");
  return (zthr_ret_t)ZEOK;
}

void ztst_thrctl(){
  int i = 0;
  zthr_id_t id;
  zthr_attr_t attr;
  sprintf(attr.name, "thr[0]");
  ZDBG("testing ztst_thrctl()...");
  zthread_create(&id, zproc_thr1, (void*)&attr);
  // main loop
  for(i=0; i< 10; i++){
    ZDBG("main loop %d...", i);
    zsleepms(500);
  }
  // cancel thread
  zthread_cancel(&id);
  zthread_join(&id);
}

void ztst_thrxctl(){
  zthr_attr_t zthr;
  zthr_attr_t zthr1;
  zthr_attr_t zthr2;
  zthr.name[0] = 0; // use default thread name.
  zthr1.name[0] = 0;
  zthr2.name[0] = 0;

  zthreadx_create(&zthr, zproc_thr2);
  zthreadx_create(&zthr1, zproc_thr2);
  zthreadx_create(&zthr2, zproc_thr2);
  zsleepsec(3);
  zthreadx_cancelall(); // cancel all threads
  zthreadx_joinall(); // join all threads
  //zthreadx_cancel(&zthr);
  //zthreadx_join(&zthr);
}
