#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/base/time.h>
#include <zit/base/error.h>
#include <stdio.h>
#include "tthread.h"

void ztst_thread(){
  ztst_semaphore();
  ztst_mutex();
  ztst_thrctl();
}

void ztst_mutex(){
  zmutex_t mtx;
  zmutex_init(&mtx);
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  if(0 == ZLOCK(&mtx)){
    ZDBG("ZLOCK() OK.");
  }else{
    ZDBG("ZLOCK() FAILED.");
  }
  if( 0 == ZUNLOCK(&mtx)){
    ZDBG("ZUNLOCK() OK.");
  }else{
    ZDBG("ZUNLOCK() FAILED.");
  }
  zmutex_uninit(&mtx);

  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  if(0 == ZLOCK(&mtx)){
    ZDBG("ZLOCK() OK.");
  }else{
    ZDBG("ZLOCK() FAILED.");
  }
  if( 0 == ZUNLOCK(&mtx)){
    ZDBG("ZUNLOCK() OK.");
  }else{
    ZDBG("ZUNLOCK() FAILED.");
  }
  
}
void ztst_semaphore(){
  zsem_t sem;
  zsem_t sem1;
  //ZMSG("test normal...");
  zsem_init(&sem,0);
  zsem_post(&sem);
  zsem_wait(&sem, ZINFINIT);
  zsem_wait(&sem, 1000);
  zsem_uninit(&sem);
  
  //ZMSG("test sem destroyed...");
  //zsem_post(&sem1);
  //zsem_init(&sem1, 0);
  //zsem_wait(&sem1,100);
  //zsem_wait(&sem1,ZINFINIT);
}

zthr_ret_t ZAPI proc_thr1(void* param){
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

zthr_ret_t ZAPI proc_thr2(void* param){
  int ret = ZEOK;
  zthr_attr_t* attr = (zthr_attr_t*)param;
  void* user_param = attr->param; // for user parameter
  ZDBG("thread[%s] running...");
  while( ZETIMEOUT == zsem_wait(&(attr->exit), 300)){
    // loop working...
    ZDBG("thread[%s] working...");
  }
  ZDBG("thread[%s] exit now.");
  return (zthr_ret_t)ret;
}


void ztst_thrctl(){
  int i = 0;
  zthr_attr_t attr;
  zthr_id_t id;
  ZDBG("testing ztst_thrctl()...");
  sprintf(attr.name, "thr[1]");
  
  zthread_create(&id, proc_thr1, (void*)&attr);
  // main loop
  for(i=0; i< 10; i++){
    ZDBG("main loop %d...", i);
    zsleepms(500);
  }
  // cancel thread
  zthread_cancel(&id);
  zthread_join(&id);
}
