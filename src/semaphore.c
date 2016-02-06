#include "export.h"
#include <zit/thread/thread_def.h>
#include <zit/thread/semaphore.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>

int zsem_init(zsem_t* sem, int value){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != (ret = sem_init(sem, 0, value))){
      ret = errno;
  }
#else//ZSYS_WINDOWS
  *sem = CreateSemaphoreA(NULL, value, ZSEM_MAX, NULL);
  if (NULL == *sem) {
	  ret = GetLastError();
  }
#endif
  ZERRC(ret);
  return ret;
}

int zsem_uninit(zsem_t* sem){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != sem_destroy(sem)){
      ret = errno;
    }
#else//ZSYS_WINDOWS
    if(0 == CloseHandle(*sem)){
      ret = GetLastError();
    }
#endif
    ZERRC(ret);
    return ret;
}

int zsem_post(zsem_t* sem){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != sem_post(sem)){
    ret = errno;
  }
#else//ZSYS_WINDOWS
  if(0 == ReleaseSemaphore(*sem, 1, NULL)){
    ret = GetLastError();
  }
#endif
  ZERRC(ret);
  return ret;
}

int zsem_wait(zsem_t* sem, int ms){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
 again:
  if(0 == ms){// trywait
    while((-1 == (ret =sem_trywait(sem))) && (errno == EINTR))continue;
    if((-1 == ret) && (EAGAIN == (ret = errno))){
     ret = ZETIMEOUT;
    }
  }else if( 0 < ms){// timed wait
    long ns;
    long sec;
    struct timespec ts;
    sec = ms/1000;
    ns = (ms%1000)*1000000;
    if( -1 == clock_gettime(CLOCK_REALTIME, &ts)){
      ret = errno;
      ZERRC(ret);
      return ret;
    }
    ts.tv_sec += sec;
    ts.tv_nsec += ns;
    ts.tv_sec += (ts.tv_nsec/ZNANO_SEC);
    ts.tv_nsec %=ZNANO_SEC;
    while((-1 == (ret = sem_timedwait(sem,&ts))) && (EINTR == errno))continue;
    if((-1 == ret) && (ETIMEDOUT == (ret = errno))){
      ret = ZETIMEOUT;
    }
  }else{// infinit wait
    while((-1 == (ret =sem_wait(sem))) && ((ret = errno) == EINTR))continue;
  }

#else//ZSYS_WINDOWS
  ret = WaitForSingleObject(*sem, ms);
  switch(ret){
  case WAIT_OBJECT_0:
    break;
  case WAIT_TIMEOUT:
    ret = ZETIMEOUT;
    break;
  case WAIT_ABANDONED:
    ret =ZEFUN_FAIL;
    break;
  case WAIT_FAILED:
    ret = GetLastError();
    break;
  default:
    ret = ZEFUN_FAIL;
    break;
  }
#endif
  if((ZEOK != ret) && (ZETIMEOUT != ret)){
    ZERRC(ret);
  }
  return ret;
}

