#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/thread/thread.h>
#include <zit/thread/jet.h>
#include <zit/base/time.h>
#include <zit/base/error.h>
#include <zit/base/queue.h>
#include <stdio.h>
#include "tthread.h"

void ztst_jet();
void ztst_queue();

void ztst_thread(){
  //ztst_semaphore();
  //  ztst_mutex();
  //ztst_thrctl();
  //ztst_thrxctl();
  ztst_queue();
  //ztst_jet();

}

int zcmp_int(zvalue_t v1, zvalue_t v2){
  int ret = ZEQUAL;
  int sub = (int)v1 - (int)v2;
  if(0 < sub){
    ret = ZGREAT;
  }else if(0 > sub){
    ret = ZLITTLE;
  }
  return ret;
}

int zact_intque(zvalue_t user, zvalue_t hint){
  int* index = (int*)hint;
  zmsg("queue[%03d] = %d", *index, user);
  ++(*index);
  return ZEOK;
}
void ztst_queue(){
  ztsk_t tsk;
  zque_t* que = NULL;
  int64_t data = 0;
  int ret = ZEOK;
  int cnt = 0;
  if(ZEOK != (ret = zqueue_create(&que))){
    ZERRC(ret);
    return;
  }
  
  zqueue_pushback(que, (zvalue_t)data);data++;
  zqueue_pushfront(que, (zvalue_t)data);data++;
  zqueue_pushback(que, (zvalue_t)data);data++;
  zqueue_pushfront(que, (zvalue_t)data);data++;
  
  tsk.hint = (void*)&cnt;
  tsk.act = zact_intque;
  zqueue_foreach(que, &tsk);

  zqueue_popfront(que,(zvalue_t*)&data);
  zqueue_popback(que, (zvalue_t*)&data);
  cnt = 0;
  zqueue_foreach(que, &tsk);
  zqueue_destroy(&que);
}

void ztst_jet(){
  zjet_init();
  zjet_run();
  zsleepsec(3);
  zjet_stop(0);
  zjet_uninit();
}
void ztst_mutex(){
  zmutex_t mtx;
  zmutex_init(&mtx);
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  zmutex_uninit(&mtx);
#ifdef ZSYS_POSIX
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
#endif
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
  zthr_t* attr = (zthr_t*)param;
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
  zthr_t* attr = (zthr_t*)param;
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
  zthr_t attr;
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
  zthr_t zthr;
  zthr_t zthr1;
  zthr_t zthr2;
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
