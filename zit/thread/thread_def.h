#ifndef _ZTHREAD_DEF_H_
#define _ZTHREAD_DEF_H_

#include <zit/base/type.h>

#ifdef ZSYS_WINDOWS

#include <windows.h>
// _beginthreadex()
typedef unsigned int zthr_ret_t;
typedef HANDLE zthr_id_t;
typedef CRITICAL_SECTION zmutex_t;
typedef HANDLE zsem_t;

#define ZAPI __stdcall
#define ZIFINIT INFINIT

#else // ZSYS_POSIX

#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
// pthread_create() compile flags:-lpthread -D_REENTRANT
typedef pthread_t zthr_id_t; // not use UINT, 64bit system will cause jion fault.
typedef void* zthr_ret_t;
typedef pthread_mutex_t zmutex_t;
typedef sem_t zsem_t;

#define ZAPI
#define ZINFINIT -1
#endif// SYS_WINDOWS

typedef zthr_ret_t (ZAPI *zproc_thr)(void*);
typedef struct _zthread_attribute_t{
  int detach;  ///< 1 detach
  int prioraty; ///< prioraty
  int result; ///< thread exit result
  zsem_t exit; ///< control thread exit
  zthr_id_t id; ///< thread id
  char name[64]; ///< thread name
  void* param; ///< user param for thread proc
}zthr_attr_t;
#if 0 // sample thread proc using zthr_attr_t normally
zthr_ret_t ZAPI proc(void* param){
  int ret = ZEOK;
  zthr_attr_t* attr = (zthr_attr_t*)param;
  void* user_param = attr->param; // for user parameter
  ZDBG("thread[%s] running...");
  while( ZETIMEDOUT == zsem_wait(&(attr->exit))){
    // loop working...
  }
  ZDBG("thread[%s] exit now.");
  return (zthr_ret_t)ret;
}
#endif

#define ZSEM_MAX 0xffff // specifies the maximum count of the semaphore.
#define ZNANO_SEC 999999999 // nanoseconds per second

#endif
