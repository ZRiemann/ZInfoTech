#include "export.h"
#include <zit/thread/thread.h>
#include <zit/thread/semaphore.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <time.h>
#include <stdlib.h>

#ifdef ZSYS_POSIX

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#else // ZSYS_WINDOWS

#include <Windows.h>
#include <process.h>

#endif // ZSYS_POSIX

zerr_t zsem_init(zsem_t* sem, int value){
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
    return ret;
}

zerr_t zsem_fini(zsem_t* sem){
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
    return ret;
}

zerr_t zsem_post(zsem_t* sem){
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
    return ret;
}

#ifdef ZSYS_WINDOWS

static zerr_t zobj_wait(HANDLE h, int ms){
    int ret = WaitForSingleObject(h, ms);
    switch(ret){
    case WAIT_OBJECT_0:ret = ZEOK;break;
    case WAIT_TIMEOUT:ret = ZETIMEOUT;break;
    case WAIT_ABANDONED:ret =ZEFUN_FAIL;break;
    case WAIT_FAILED:ret = GetLastError();break;
    default:ret = ZEFUN_FAIL;break;
    }
    return ret;
}
#endif

zerr_t zsem_wait(zsem_t* sem, int ms){
    int ret = ZEOK;
    //ZDBG("sem_wait begin...");
#ifdef ZSYS_POSIX
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
        ts.tv_nsec %= ZNANO_SEC;
        ts.tv_nsec += ns;
        ts.tv_sec += (ts.tv_nsec/ZNANO_SEC);
        ts.tv_nsec %=ZNANO_SEC;
        while((-1 == (ret = sem_timedwait(sem,&ts))) && (EINTR == errno))continue;
        if((-1 == ret) && (ETIMEDOUT == (ret = errno))){
            ret = ZETIMEOUT;
        }
    }else{// infinit wait
        //while((-1 == (ret =sem_wait(sem))) && ((ret = errno) == EINTR))continue;
        ret = sem_wait(sem);
        if(-1 == ret ){
            ret = errno;
        }
    }

#else//ZSYS_WINDOWS
    ret = zobj_wait(*sem, ms);
#endif
    return ret;
}

zerr_t zsem_getvalue(zsem_t* sem, int* value){
    int ret = ZEOK;
#ifdef ZSYS_POSIX
    if(0 != sem_getvalue(sem, value)){
        ret = errno;
    }
#else // ZSYS_WINDOWS
    ret = zobj_wait(*sem, 0);
    if(ZEOK == ret){
        if(0 == ReleaseSemaphore(*sem, 1, value)){
            ret = GetLastError();
        }
        (*value)++;
    }else{
        *value = 0;
    }
#endif
    return ret;
}

/*============================================================================*/
/*
 * implement thread
 */
ZAPI zerr_t zthread_create(zthr_id_t* id, zproc_thr proc, void* param){
    zerr_t ret = ZEOK;
#ifdef ZSYS_POSIX
    if( 0 != (ret = pthread_create(id, NULL, proc, param)))ret = errno;
#else//ZSYS_WINDOWS
    if( NULL == (*id = (zthr_id_t)_beginthreadex(NULL, 0, proc, param, 0, NULL)))ret = GetLastError();
#endif
    ZDBG("zthread_create[id:%p] %s", id, zstrerr(ret));
  return ret;
}

ZAPI zerr_t zthread_detach(zthr_id_t* id){
      zerr_t ret = ZEOK;
#ifdef ZSYS_POSIX
      if(0 != (ret = pthread_detach(*id)))ret = errno;
#else//ZSYS_WINDOWS
      if(0 == CloseHandle(*id))ret = GetLastError();
#endif
      ZDBG("zthread_detach[id:%p] %s", id, zstrerr(ret));
  return ret;
}

ZAPI zerr_t zthread_join(zthr_id_t* id){
      zerr_t ret = ZEOK;
      void* result = NULL;
      ZMSG("zthread_join(id:%p) begin...", id);
#ifdef ZSYS_POSIX
      if( 0 != (ret = pthread_join(*id, &result)))ret = errno;
#else//ZSYS_WINDOWS
	  ret = WaitForSingleObject(*id, ZINFINITE);
	  switch (ret){
	  case WAIT_OBJECT_0:ret = ZEOK; break;
	  case WAIT_TIMEOUT:ret = ZETIMEOUT; break;
	  case WAIT_ABANDONED:ret = ZEFUN_FAIL; break;
	  case WAIT_FAILED:ret = GetLastError(); break;
	  default:ret = ZEFUN_FAIL; break;
	  }
      if(0 == CloseHandle(*id))ret = GetLastError();
#endif
      ZMSG("zthread_join(id:%p) end %s", id, zstrerr(ret));
      return ret;
}


ZAPI zerr_t zthread_cancel(zthr_id_t* id){
    zerr_t ret = ZEOK;
    ZMSG("zthread_cancel(id:%p) begin...", id);
#ifdef ZSYS_POSIX
    if(0 != (ret = pthread_cancel(*id)))ret = errno;
#else//ZSYS_WINDOWS
    ret = TerminateThread(*id, 0) ? ZOK : ZFUN_FAIL;
#endif
    ZMSG("zthread_cancel(id:%p) end %s", id, zstrerr(ret));
    return ret;
}

