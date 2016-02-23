#include "export.h"
#include <zit/thread/mutex.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <stdlib.h>

int zmutex_init(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_init(pmtx, 0);
  if( 0 != ret )
  {
    ret = ZEFUN_FAIL;
  }
#else//ZSYS_WINDOWS
  InitializeCriticalSection(pmtx);
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

int zmutex_uninit(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_destroy(pmtx);
  if( 0 != ret ){
    ret = ZEFUN_FAIL;
  }
#else//ZSYS_WINDOWS
  DeleteCriticalSection(pmtx);
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

ZEXP zmutex_t* zmutex_create(){
  zmutex_t* mtx;
  if(NULL != (mtx = (zmutex_t*)malloc(sizeof(zmutex_t)))){
#ifdef ZSYS_POSIX
    if( 0 != pthread_mutex_init(mtx, 0)){
      ZERRC(ZEFUN_FAIL);
    }
#else//ZSYS_WINDOWS
    InitializeCriticalSection(mtx);
#endif    
  }else{
    ZERRC(ZEMEM_INSUFFICIENT);
  }
  return mtx;
}

ZEXP void zmutex_destroy(zmutex_t* mtx){
  if(NULL == mtx)
    return;
#ifdef ZSYS_POSIX
  if( 0 != pthread_mutex_destroy(mtx)){
    ZERRC(ZEFUN_FAIL);
  }
#else//ZSYS_WINDOWS
  DeleteCriticalSection(mtx);
#endif
}

int zmutex_lock(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_lock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  EnterCriticalSection(pmtx);
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

int zmutex_unlock(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_unlock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  LeaveCriticalSection(pmtx);
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

int zmutex_trylock(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_trylock(pmtx);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = TryEnterCriticalSection(pmtx);
  if( 0 == ret){
    ret = ZEAGAIN;
  }
#endif
  return ret;
}
