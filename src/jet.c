/**@file jet.c
   @brief general thread pool
   @note
   mission: 1        2         3        4        5 ...     misnum
            ^id=++id%misnum
*/
#include "export.h"
#include <zit/base/trace.h>
#include <zit/base/queue.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/thread/thread.h>
#include <zit/thread/jet.h>
#include <stdlib.h>

#define ZJET_MAX_MISSION 128

typedef struct zmission_s{
  zsem_t sem; ///< semaphore
  zmutex_t mtx; ///< mutex for push task
  zthr_t thr; ///< thread
  zque_t* tsks; ///< task queue
  znod_t nod; ///< nod.value = mission id
}zmis_t;

typedef struct zjet_s{
  int misnum;
  int misnow;
  zmis_t mis[ZJET_MAX_MISSION]; ///< missions
  zque_t* idels; ///< idel task queue
  zvalue_t id; ///< id generator
  zmutex_t mtx; ///< jet lock
  zsem_t semidel; ///< idel task semaphore
}zjet_t;

zjet_t* zg_jet = NULL;


zthr_ret_t ZAPI zproc_mission(void* param){
  int ret = ZEOK;
  
  zmis_t* mis = (zmis_t*)param; // for user parameter
  zthr_t* thr = &(mis->thr);
  ztsk_t* tsk = NULL;
  if(ZEOK != zthreadx_procbegin(thr)){
    zthreadx_procend(thr, ret);
    return (zthr_ret_t)ret;
  }
  while(ZETIMEOUT == zsem_wait(&(thr->exit), 0)){
    if(ZEOK == zsem_wait(&(mis->sem), 300)){ // get normal task
      zqueue_popfront(mis->tsks, (zvalue_t*)&tsk);
    }else if(ZEOK == zsem_wait(&(zg_jet->semidel), 0)){ // get idel task
      zmutex_lock(&(zg_jet->mtx));
      zqueue_popfront(zg_jet->idels, (zvalue_t*)&tsk);
      zmutex_unlock(&(zg_jet->mtx));
    }else{
      continue; // do nothing
    }
    // act task
    tsk->act(tsk->user, tsk->hint);
    tsk->free(tsk->user, tsk->hint);
  }
  zthreadx_procend(thr, ret);
  return (zthr_ret_t)ret;
}

int zjet_init(){
  int ret = ZEOK;
  int i = 0;
  zg_jet = (zjet_t*)malloc(sizeof(zjet_t));
  if( NULL == zg_jet){
    ret = ZEMEM_INSUFFICIENT;
  }else{
    zg_jet->idels = NULL;
    zg_jet->id = 0;
    zmutex_init(&(zg_jet->mtx));
    zg_jet->misnum = 8; ///< cpu*2+2
    for(i=0; i<ZJET_MAX_MISSION; i++){
      zg_jet->mis[i].nod.value = (zvalue_t)i; ///< set mission id
    }
  }
  ZERRC(ret);
  return ret;
}

int zjet_uninit(){
  int ret = ZENOT_SUPPORT;
  // mast stop first
  
  ZERRC(ret);
  return ret;
}

int zjet_run(){
  int ret = ZEOK;
  // run thread pool
  //zthreadx_create();
  ZERRC(ret);
  return ret;
}

int zjet_stop(int flag){
  int ret = ZENOT_SUPPORT;
  // stop all buffered thread
  
  ZERRC(ret);
  return ret;
}

int zjet_assign(ztsk_t* tsk){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}

int zjet_getid(int* id){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}
