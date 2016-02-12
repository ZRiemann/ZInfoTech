#include "export.h"
#include <zit/thread/mutex.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>

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
  ZERRC(ret);
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
  ZERRC(ret);
  return ret;
}

int zmutex_lock(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_lock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  EnterCriticalSection(pmtx);
#endif
  ZERRC(ret);
  return ret;
}

int zmutex_unlock(zmutex_t* pmtx){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_mutex_unlock(pmtx);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  LeaveCriticalSection(pmtx);
#endif
  ZERRC(ret);
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
