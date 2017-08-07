#ifndef _ZTHREAD_THREAD_H_
#define _ZTHREAD_THREAD_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>

#ifdef ZSYS_POSIX

#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>

#define ZCALL
#define ZINFINITE -1

typedef pthread_t zthr_id_t; // not use UINT, 64bit system will cause jion fault.
typedef void* zthr_ret_t;

#else // ZSYS_WINDOWS

#include <Windows.h>
#include <process.h>

#define ZCALL __stdcall
#define ZINFINITE INFINITE

typedef unsigned int zthr_ret_t;
typedef HANDLE zthr_id_t;

#endif // ZSYS_POSIX

typedef zthr_ret_t (ZCALL *zproc_thr)(void*);

zerr_t zthread_create(zthr_id_t* id, zproc_thr proc, void* param); 
zerr_t zthread_detach(zthr_id_t* id);
zerr_t zthread_join(zthr_id_t* id);
zerr_t zthread_cancel(zthr_id_t* id); // not release system resource.
uint32_t zthread_self();

#include <zit/base/error.h>
#include <zit/base/trace.h>

inline zerr_t zthread_create(zthr_id_t* id, zproc_thr proc, void* param){
    zerr_t ret = ZEOK;
#ifdef ZSYS_POSIX
    if( 0 != (ret = pthread_create(id, NULL, proc, param)))ret = errno;
#else//ZSYS_WINDOWS
    if( NULL == (*id = (zthr_id_t)_beginthreadex(NULL, 0, proc, param, 0, NULL)))ret = GetLastError();
#endif
    ZDBG("zthread_create[id:%p] %s", id, zstrerr(ret));
  return ret;
}

inline zerr_t zthread_detach(zthr_id_t* id){
      zerr_t ret = ZEOK;
#ifdef ZSYS_POSIX
      if(0 != (ret = pthread_detach(*id)))ret = errno;
#else//ZSYS_WINDOWS
      if(0 == CloseHandle(*id))ret = GetLastError();
#endif
      ZDBG("zthread_detach[id:%p] %s", id, zstrerr(ret));
  return ret;
}

inline zerr_t zthread_join(zthr_id_t* id){
      zerr_t ret = ZEOK;
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


inline zerr_t zthread_cancel(zthr_id_t* id){
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

inline uint32_t zthread_self(){
#ifdef ZSYS_WINDOWS
    return (uint32_t)GetCurrentThreadId();
#else
    return (uint32_t)syscall(SYS_gettid);
#endif
}

#endif
