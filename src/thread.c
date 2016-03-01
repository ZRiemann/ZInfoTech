#include "export.h"
#include <zit/thread/thread_def.h>
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <stdio.h>
#include <zit/thread/thread_def.h>

static zthr_t* zg_thr_head = NULL; ///< head ofthread attr list
static zmutex_t zg_thr_mtx; ///< lock for thread attr list
static int zg_thr_name = 0; ///< thread name base thr0,thr1,...
static int zthr_listpush(zthr_t* attr){
  int ret = ZEOK;
  if( NULL == zg_thr_head ){
    // first thread not need lock, and init list
    if( ZEOK != (ret = zmutex_init(&zg_thr_mtx)))return ret;
    zg_thr_head = attr;
    attr->next = NULL;
    ++zg_thr_name;
  }else{
    zmutex_lock(&zg_thr_mtx);
    attr->next = zg_thr_head;
    zg_thr_head = attr;
    ++zg_thr_name;
    zmutex_unlock(&zg_thr_mtx);
  }
  return ret;
}

static int zthr_listerase(zthr_t* attr){
  int ret = ZENOT_EXIST;
  zthr_t* pa = zg_thr_head;
    zmutex_lock(&zg_thr_mtx);
    attr->detach = 1;
    if(pa == attr){
      zg_thr_head = pa->next;
      ret = ZEOK;
    }
    while(pa && (pa->next != attr))pa = pa->next;
    if(pa){
      pa->next = attr->next;
      ret = ZEOK;
    }
    zmutex_unlock(&zg_thr_mtx);

    if(NULL == zg_thr_head){
      zmutex_uninit(&zg_thr_mtx);
    }
    ZERRCX(ret);
    return ret;
}

// normal thread control
int zthread_create(zthr_id_t* id, zproc_thr proc, void* param){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_create(id, NULL, proc, param)))ret = errno;
#else//ZSYS_WINDOWS
  if( NULL == (*id = (zthr_id_t)_beginthreadex(NULL, 0, proc, param, 0, NULL)))ret = GetLastError();
#endif
  ZDBG("zthread_create[id:%p] %s", id, zstrerr(ret));
  return ret;

} 

int zthread_detach(zthr_id_t* id){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_detach(*id)))ret = errno;
#else//ZSYS_WINDOWS
  if(0 == CloseHandle(*id))ret = GetLastError();
#endif
  ZDBG("zthread_detach[id:%p] %s", id, zstrerr(ret));
  return ret;
}

int zthread_join(zthr_id_t* id){
  int ret = ZEOK;
  void* result = NULL;
  ZMSG("zthread_join(id:%p) begin...", id);
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_join(*id, &result)))ret = errno;
#else//ZSYS_WINDOWS
  ret = zobj_wait(*id, ZINFINITE);
#endif
  ZMSG("zthread_join(id:%p) end %s", id, zstrerr(ret));
  return ret;
}

int zthread_cancel(zthr_id_t* id){
  int ret = ZEOK;
  ZMSG("zthread_cancel(id:%p) begin...", id);
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_cancel(*id)))ret = errno;
#else//ZSYS_WINDOWS
  // dummy, just return ZEOK.
#endif
  ZMSG("zthread_cancel(id:%p) end %s", id, zstrerr(ret));
  return ret;
}

// threads manager
int zthreadx_create(zthr_t* attr, zproc_thr proc){
  int ret = ZEOK;
  attr->join = 0;
  attr->detach = 0;
  attr->next = NULL;
  if(0 == attr->name[0]){
    sprintf(attr->name,"thread[%d]",zg_thr_name);
  }
  zthr_listpush(attr);
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_create(&(attr->id), NULL, proc, (void*)attr)))ret = errno;
#else//ZSYS_WINDOWS
  if( NULL == (attr->id = (zthr_id_t)_beginthreadex(NULL, 0, proc, (void*)attr, 0, NULL)))ret = GetLastError();
#endif
  if(ZEOK != ret){
    zthr_listerase(attr);
  }

  ZDBG("%s create %s", attr->name, zstrerr(ret));
  return ret;
} 

int zthreadx_detach(zthr_t* attr){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_detach(attr->id)))ret = errno;
#else//ZSYS_WINDOWS
  if(0 == CloseHandle(attr->id))ret = GetLastError();
#endif
  if(ZEOK == ret){
    zthr_listerase(attr);
    attr->detach = 1;
  }
  ZDBG("%s detach %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_join(zthr_t* attr){
  int ret = ZEOK;
  void* result = NULL;
  ZMSG("%s join begin...", attr->name);
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_join(attr->id, &result)))ret = errno;
#else//ZSYS_WINDOWS
  ret = zobj_wait(attr->id, ZINFINITE);
#endif
  if(ZEOK == ret){
    zthr_listerase(attr);
    attr->join = 1;
    //zsem_uninit(&(attr->exit)); // destroyed in thread proc.
  }
  ZMSG("%s join end %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_cancel(zthr_t* attr){
  int ret = ZEOK;
  while( 1 != attr->run){
    zsleepms(0);
  }
  ret = zsem_post(&(attr->exit));
  ZMSG("%s cancel %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_cancelall(){
  int ret = ZEOK;
  zthr_t* pa = NULL;
  zmutex_lock(&zg_thr_mtx);
  pa = zg_thr_head;
  while(pa){
    ZMSG("%s cancel",pa->name);
    zsem_post(&(pa->exit));
    pa = pa->next;
  }
  zmutex_unlock(&zg_thr_mtx);
  return ZEOK;
}
int zthreadx_joinall(){
  int ret = ZEOK;
  void* result = NULL;
  zthr_t* pa = NULL;
  zmutex_lock(&zg_thr_mtx);
  pa = zg_thr_head;
  while(pa){
    //zthreadx_join(pa); // cause dead lock by mutex
    ZMSG("%s join begin...", pa->name);
#ifdef ZSYS_POSIX
    if( 0 != (ret = pthread_join(pa->id, &result)))ret = errno;
#else // ZSYS_WINDOWS
    ret = zobj_wait(pa->id, ZINFINITE);
#endif
    ZMSG("%s join end. %s", pa->name, zstrerr(ret));
    pa = pa->next;
  }
  zg_thr_head = NULL;
  zmutex_unlock(&zg_thr_mtx);
  
  zmutex_uninit(&zg_thr_mtx);
  return ZEOK;
}

ZEXP int zthreadx_procbegin(zthr_t* attr){
  int ret = zsem_init(&(attr->exit), 0); //* init semaphore.
  attr->run = 1;
  ZMSG("%s begin...", attr->name);
  return ret;
}

ZEXP int zthreadx_procend(zthr_t* attr, int ret){
  attr->result = ret;
  ZMSG("%s end.", attr->name);
  return zsem_uninit(&attr->exit); //* destroy semaphore.
}
