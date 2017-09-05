#ifndef _ZTHREAD_SPIN_H_
#define _ZTHREAD_SPIN_H_
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
/**
 * @file zit/thread/spin.h
 * @brief a spin lock wrapper
 * @author Z.Riemann
 * @date 2017-08-01
 */

#include <zit/base/platform.h>
#include <zit/base/error.h>

#ifdef ZSYS_WINDOWS
#include <Windows.h>
typedef CRITICAL_SECTION zspinlock_t;

#define zspin_init(spin) (InitializeCriticalSectionEx(spin, 8192, CRITICAL_SECTION_NO_DEBUG_INFO) - 1)
#define zspin_fini(spin) DeleteCriticalSection(spin)
#define zspin_lock(spin) EnterCriticalSection(spin)
#define zspin_unlock(spin) LeaveCriticalSection(spin)
#define zspin_trylock(spin) (TryEnterCriticalSection(spin) - 1)

#else // ZSYS_WINDOWS

#include <pthread.h>
typedef pthread_spinlock_t zspinlock_t;

#define zspin_init(spin) pthread_spin_init(spin, PTHREAD_PROCESS_PRIVATE)
#define zspin_fini(spin) pthread_spin_destroy(spin)
#define zspin_lock(spin) pthread_spin_lock(spin)
#define zspin_unlock(spin) pthread_spin_unlock(spin)
#define zspin_trylock(spin) pthread_spin_trylock(spin)

#endif //ZSYS_WINDOWS

#define zspin_xchg(spin, v1, v2) do{zspin_lock(spin);\
        v1 ^= v2; v2 ^= v1; v1 ^= v2; zspin_unlock(spin);}while(0)
#endif
