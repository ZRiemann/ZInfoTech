#ifndef _ZTHREAD_MUTEX_H_
#define _ZTHREAD_MUTEX_H_
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

#ifdef ZSYS_WINDOWS
#include <Windows.h>
typedef CRITICAL_SECTION zmtx_t;

#define zmtx_init(mtx) (InitializeCriticalSectionEx(mtx, 8192, CRITICAL_SECTION_NO_DEBUG_INFO) ? ZEOK : ZEFUN_FAIL)
#define zmtx_fini(mtx) DeleteCriticalSection(mtx)
#define zmtx_lock(mtx) EnterCriticalSection(mtx)
#define zmtx_unlock(mtx) LeaveCriticalSection(mtx)
#define zmtx_trylock(mtx) (TryEnterCriticalSection(mtx) ? ZEOK : ZEFUN_FAIL)

#else // ZSYS_WINDOWS

#include <pthread.h>
typedef pthread_mutex_t zmtx_t;

#define zmtx_init(mtx) (pthread_mutex_init(mtx, 0) ? ZEFUN_FAIL : ZEOK)
#define zmtx_fini(mtx) (pthread_mutex_destroy(mtx) ? ZEFUN_FAIL : ZEOK)
#define zmtx_lock(mtx) pthread_mutex_lock(mtx)
#define zmtx_unlock(mtx) pthread_mutex_unlock(mtx)
#define zmtx_trylock(mtx) (pthread_mutex_trylock(mtx) ? ZEFUN_FAIL : ZEOK)

#endif //ZSYS_WINDOWS

#endif
