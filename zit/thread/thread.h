#ifndef _ZTHREAD_THREAD_H_
#define _ZTHREAD_THREAD_H_
/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
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
