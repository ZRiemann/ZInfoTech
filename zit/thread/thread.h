#ifndef _ZTHREAD_THREAD_H_
#define _ZTHREAD_THREAD_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>

ZC_BEGIN

typedef uint32_t ztid_t;  // thread_self()

#ifdef ZSYS_POSIX
#include <pthread.h>
#include <sys/syscall.h>
#define ZCALL
#define ZINFINITE -1
typedef pthread_t zthr_id_t; // not use UINT, 64bit system will cause jion fault.
typedef void* zthr_ret_t;
#define zthread_self() (ztid_t)syscall(SYS_gettid)
#else
#include <windows.h>
#define ZCALL __stdcall
#define ZINFINITE INFINITE
typedef unsigned int zthr_ret_t;
typedef HANDLE zthr_id_t;
#define zthread_self() (ztid_t)GetCurrentThreadId()
#endif

typedef zthr_ret_t (ZCALL *zproc_thr)(void*);

ZAPI zerr_t zthread_create(zthr_id_t* id, zproc_thr proc, void* param); 
ZAPI zerr_t zthread_detach(zthr_id_t* id);
ZAPI zerr_t zthread_join(zthr_id_t* id);
ZAPI zerr_t zthread_cancel(zthr_id_t* id); // not release system resource.

ZC_END

#endif
