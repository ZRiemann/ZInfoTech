#ifndef _ZTHREAD_DEF_H_
#define _ZTHREAD_DEF_H_

#include <zit/base/type.h>

#ifdef ZSYS_WINDOWS

#include <windows.h>
// _beginthreadex()
typedef unsigned int zthr_ret_t;
typedef unsigned int zthr_id_t;
typedef zthr_ret_t (__stdcall *zproc_thr)(void*);

typedef CRITICAL_SECTION zmutex_t;
#else // ZSYS_POSIX

#include <pthread.h>
// pthread_create() compile flags:-lpthread -D_REENTRANT
typedef pthread_t zthr_id_t; // not use UINT, 64bit system will cause jion fault.
typedef void* zthr_ret_t;
typedef zthr_ret_t (*zproc_thr)(void*); 

typedef pthread_mutex_t zmutex_t;
#endif// SYS_WINDOWS

typedef enum
{
  ZTHR_NOTSUPPORT = 1,     // not support thread
  ZTHR_ONLYSUPPORT = 2,    // support thread, not support priority
  ZTHR_SUPPORTPRI = 3      // support priority
}zthr_ability_t;

#endif
