#include "export.h"
#include <zit/thread/rwlock.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <stdlib.h>

int zrwlock_init(zrwlock_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_init(pmtx, 0);
  if( 0 != ret )
  {
    ret = ZEFUN_FAIL;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

int zrwlock_fini(zrwlock_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_destroy(pmtx);
  if( 0 != ret ){
    ret = ZEFUN_FAIL;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
#if ZTRACE_MUTEX
  ZERRC(ret);
#endif
  return ret;
}

int zrwlock_rdlock(zrwlock_t* pmtx){
  int ret = ZEOK;
#if ZTRACE_MUTEX
  zdbg("RWLOCK: begin lock<%p>", pmtx);
#endif

#ifdef ZSYS_POSIX
  ret = pthread_rwlock_rdlock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
#if ZTRACE_MUTEX
  zdbg("RWLOCK: end lock<%p>", pmtx);
  //ZERRC(ret);
#endif
  return ret;
}

int zrwlock_tryrdlock(zrwlock_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_tryrdlock(pmtx);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}
int zrwlock_wrlock(zrwlock_t* pmtx){
  int ret = ZEOK;
#if ZTRACE_MUTEX
  zdbg("RWLOCK: begin lock<%p>", pmtx);
#endif

#ifdef ZSYS_POSIX
  ret = pthread_rwlock_wrlock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
#if ZTRACE_MUTEX
  zdbg("RWLOCK: end lock<%p>", pmtx);
  //ZERRC(ret);
#endif
  return ret;
}

int zrwlock_trywrlock(zrwlock_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_trywrlock(pmtx);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}

ZAPI int zrwlock_timedrdlock(zrwlock_t* mtx, int sec){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  struct timespec spec;
  spec.tv_sec = sec;
  spec.tv_nsec = 0;
  ret = pthread_rwlock_timedrdlock(mtx, &spec);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}

ZAPI int zrwlock_timedwrlock(zrwlock_t* mtx, int sec){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  struct timespec spec;
  spec.tv_sec = sec;
  spec.tv_nsec = 900000000;

  ret = pthread_rwlock_timedwrlock(mtx, &spec);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}

int zrwlock_unlock(zrwlock_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_unlock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  ret = ZNOT_SUPPORT;
#endif
#if ZTRACE_MUTEX
  //ZERRC(ret);
  zdbg("RWLOCK: unlock<%p>", pmtx);
#endif
  return ret;
}

/*
zrwlock_t* zrwlock_create(){
  zrwlock_t* mtx;
  if(NULL != (mtx = (zrwlock_t*)malloc(sizeof(zrwlock_t)))){
#ifdef ZSYS_POSIX
    if( 0 != pthread_rwlock_init(mtx, 0)){
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

void zrwlock_destroy(zrwlock_t* mtx){
  if(NULL == mtx)
    return;
#ifdef ZSYS_POSIX
  if( 0 != pthread_rwlock_destroy(mtx)){
    ZERRC(ZEFUN_FAIL);
  }
#else//ZSYS_WINDOWS
  DeleteCriticalSection(mtx);
#endif
}
*/
