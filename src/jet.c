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
#include <string.h>

#define ZJET_MAX_MISSION 128

typedef struct zmission_s{
  int id; ///< mission id
  zsem_t sem; ///< semaphore
  zmtx_t mtx; ///< mutex for push task
  zthr_t thr; ///< thread
  zque_t tsks; ///< task queue
  //znod_t nod; ///< nod.value = mission id
}zmis_t;

typedef struct zjet_s{
  int misnum; ///< init missions run
  int misnow; ///< now miss = init miss + extends miss
  zmis_t mis[ZJET_MAX_MISSION]; ///< missions
  zque_t idels; ///< idel task queue
  zvalue_t id; ///< id generator
  zmtx_t mtx; ///< lock run/stop/push task to idels
  zmtx_t mtxpop; ///< lock pop task from idels
  zsem_t semidel; ///< idel task semaphore
  zstatus_t status; ///< jet status
}zjet_t;

zjet_t* zg_jet = NULL;


zthr_ret_t ZAPI zproc_mission(void* param){
  int ret = ZEOK;
  zthr_ret_t ret_ = 0;
  zthr_t* thr = (zthr_t*)param;
  zmis_t* mis = (zmis_t*)thr->param;
  ztsk_t* tsk = NULL;
  zsem_t* exit = &thr->exit;
  zsem_t* semtsk = &(mis->sem);
  zsem_t* semidel = &(zg_jet->semidel);
  zmutex_t* mtxidel = &(zg_jet->mtxpop);
  zque_t* idels = &(zg_jet->idels);
  zque_t* tsks = &(mis->tsks);
  
  if(ZEOK != (ret = zthreadx_procbegin(thr))){
    zthreadx_procend(thr, ret);
    ZCONVERT(ret_, ret);
    return ret_;
  }
  while(ZETIMEOUT == zsem_wait(exit, 0)){
    if(ZEOK == zsem_wait(semtsk, 300)){ // get normal task
      zqueue_popfront(tsks, (zvalue_t*)&tsk);
    }else if(ZEOK == zsem_wait(semidel, 0)){ // get idel task
      zmutex_lock(mtxidel);
      zqueue_popfront(idels, (zvalue_t*)&tsk);
      zmutex_unlock(mtxidel);
    }else{
      continue; // do nothing
    }
    // act task
    tsk->act(tsk->user, tsk->hint);
    tsk->free(tsk->user, tsk->hint);
  }
  zthreadx_procend(thr, ret);
  ZCONVERT(ret_, ret);
  return ret_;
}

int zjet_init(){
  int ret = ZEOK;
  int i = 0;

  if(NULL == zg_jet){ // may init by other module, one jet per process
    zg_jet = (zjet_t*)malloc(sizeof(zjet_t));
    if(NULL == zg_jet){
      ret = ZEMEM_INSUFFICIENT;
    }else{
      zg_jet->id = 0;
      zmutex_init(&(zg_jet->mtx));
      zmutex_init(&(zg_jet->mtxpop));
      zqueue_init(&(zg_jet->idels));
      zsem_init(&(zg_jet->semidels));
      zg_jet->misnum = 8; ///< task delay: cpu*2+2
      for(i=0; i<ZJET_MAX_MISSION; i++){
	//ZCONVERT(i, zg_jet->mis[i].nod.value);
	zg_jet->mis[i].id = i;
      }
      zg_jet->status = ZINIT;
    }
  }
  
  ZERRC(ret);
  return ret;
}

int zjet_uninit(){
  int ret = ZEOK;
  // mast stop first
  if(NULL != zg_jet){
    if(ZRUN == zg_jet->status)zjet_stop(0);
    zqueue_uninit(&(zg_jet->idels));
    zmutex_uninit(&(zg_jet->mtx));
    zmutex_uninit(&(zg_jet->mtxpop));
    zsem_uninit(&(zg_jet->semidels));
    free(zg_jet);
    zg_jet = NULL;
  }
  ZERRC(ret);
  return ret;
}

static int _zjet_misrun(zmis_t* mis){
  // run a mission, locked by zg_jet->mtx
  int ret = ZEOK;
  do{
    if(NULL == mis){ret = ZEPARAM_INVALID;break;}
    // Init mission.
    //mis->id;  //has inited 
    zsem_init(&(mis->sem));
    zmutex_init(&(mis->mtx));
    zqueue_init(&(mis->tsks));  
    // Set thread attribute.
    sprintf(mis->thr.name, "thr_mis[%d]", mis->id);
    mis->thr.param = (void*)mis;
    // Run thread.
    if(ZEOK != (ret = zthreadx_create(&(mis->thr), zproc_mission)){break;}
  }while(0);
  ZERRC(ret);
  return ret;
}
static int _zjet_misstop(zmis_t* mis, int flag){
  // stop a mission
  int ret = ZENOT_SUPPORT;
  flag = flag;
  // task delay...
  ZERRC(ret);
  return ret;
}
int zjet_run(){
  // run thread pool
  int ret = ZEOK;
  int i = 0;
  
  if(NULL == zg_jet){
    ret = ZENOT_INIT;ZERRC(ret);return ret;
  }
  zmutex_lock(&(zg_jet->mtx));
  if((zg_jet->status == ZINIT) || (zg_jet->status == ZSTOP)){
    for(i = 0; i < zg_jet->misnum; i++){
      if( ZEOK != _zjet_misrun(&zg_jet->mis[i]) ){
	zg_jet->misnum = i+1; // if run mission fail break. and run
	zg_jet->misnow = i+1;
	break;
      }
    }
    zg_jet->status = ZRUN;
  }
  zmutex_unlock(&(zg_jet->mtx));
  
  ZERRC(ret);
  return ret;
}

int zjet_stop(int flag){
  int ret = ZEOK;
  int i = 0;

  // stop all buffered thread
  if(NULL == zg_jet){
    ret = ZENOT_INIT;ZERRC(ret);return ret;
  }
  zmutex_lock(&(zg_jet->mtx));
  if(zg_jet->status == ZRUN){
    for(i = 0; i < zg_jet->misnow; i++){
      _zjet_misstop(&zg_jet->mis[i], flag);
    }
    zg_jet->status = ZSTOP;
  }
  zmutex_unlock(&(zg_jet->mtx));
  
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
