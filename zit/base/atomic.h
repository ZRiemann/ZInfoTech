#ifndef _ZBASE_ATOMIC_H_
#define _ZBASE_ATOMIC_H_
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
#include "inttypes.h"
typedef volatile long zatm32_t;
typedef volatile __int64 zatmv64_t;
typedef long* zatm_t;
typedef __int64* zatm64_t;
#elif defined(ZSYS_POSIX)
#include <inttypes.h>
typedef volatile int32_t zatm32_t;
typedef volatile int64_t zatmv64_t;
typedef int32_t *zatm_t;
typedef int64_t *zatm64_t;
#endif//ZSYS_WINDOWS

#include <malloc.h>

typedef void* zatmp_t;

#ifdef ZSYS_WINDOWS
#define zmem_align(alignment, size) _aligned_malloc(size,alignment)
#define zmem_align64(size) _aligned_malloc(size, 64)
#define zmem_align_free(ptr) _aligned_free(ptr)
#else // ZSYS_POSIX
#define zmem_align(alignment, size) memalign(alignment, size)
#define zmem_align64(size) memalign(64, size)
#define zmem_align_free(ptr) free(ptr)
#endif

#ifdef ZSYS_POSIX
// gcc
#define zatm_alloc(size) memalign(64, size)
#define zatm_alloc_atm() (zatm_t)memalign(64, 32)
#define zatm_alloc_atm64() (zatm64_t)memalign(64, 64)
#define zatm_free(ptr) free(ptr)
#define zatm_add(ptr, value) __sync_add_and_fetch(ptr, value)
#define zatm_sub(ptr, value) __sync_sub_and_fetch(ptr, value)
#define zatm_inc(ptr) __sync_add_and_fetch(ptr, 1)
#define zatm_dec(ptr) __sync_sub_and_fetch(ptr, 1)
#define zatm_cas(ptr, oldval, newval) __sync_val_compare_and_swap(ptr,oldval,newval)
#define zatm_bcas(ptr, oldval, newval) __sync_bool_compare_and_swap(ptr,oldval,newval)
#define zatm_xchg(ptr, newval) __sync_lock_test_and_set (ptr, newval);
#define zatm_xchg_ptr(ptr, newval) __sync_lock_test_and_set (ptr, newval);

#define zatm_barrier() __sync_synchronize()

#define zatm64_add(ptr, value) __sync_add_and_fetch(ptr, value)
#define zatm64_sub(ptr, value) __sync_sub_and_fetch(ptr, value)
#define zatm64_inc(ptr) __sync_add_and_fetch(ptr, 1)
#define zatm64_dec(ptr) __sync_sub_and_fetch(ptr, 1)
#define zatm64_cas(ptr, oldval, newval) __sync_val_compare_and_swap(type *ptr,oldval,newval)
#define zatm64_xchg(ptr, newval) __sync_lock_test_and_set (ptr, newval);
#define zatm64_xchg_ptr(ptr, newval) __sync_lock_test_and_set (ptr, newval);

#elif defined(ZSYS_WINDOWS)

#include <Windows.h>
#define zatm_alloc(size) _aligned_malloc(size, 64)
#define zatm_alloc_atm() (zatm_t)_aligned_malloc(32, 64)
#define zatm_alloc_atm64() (zatm64_t)_aligned_malloc(64, 64)
#define zatm_free(ptr) _aligned_free(ptr)
#define zatm_add(ptr, value) InterlockedExchangeAdd(ptr, value)
#define zatm_sub(ptr, value) InterlockedExchangeAdd(ptr, -(value))
#define zatm_inc(ptr) InterlockedIncrement(ptr)
#define zatm_dec(ptr) InterlockedDecrement(ptr)
#define zatm_cas(ptr, oldval, newval) InterlockedCompareExchange(ptr, newval, oldval)
#define zatm_bcas(ptr, oldval, newval) (oldval == InterlockedCompareExchange(ptr, newval, oldval))
#define zatm_xchg(ptr, newval) InterlockedExchange(ptr, newval)
#define zatm_xchg_ptr(ptr, newval) InterlockedExchangePointer((volatile PVOID*)ptr, (PVOID)newval)

#define zatm64_add(ptr, value) InterlockedExchangeAdd64(ptr, value)
#define zatm64_sub(ptr, value) InterlockedExchangeAdd64(ptr, -(value))
#define zatm64_inc(ptr) InterlockedIncrement64(ptr)
#define zatm64_dec(ptr) InterlockedDecrement64(ptr)
#define zatm64_cas(ptr, oldval, newval) InterlockedCompareExchange64(ptr, newval, oldval)
#define zatm64_xchg(ptr, newval) InterlockedExchange64(ptr, newval)
#define zatm_barrier()

#else // some embend code, no threads

#define zatm_alloc(size) cmalloc(1, size)
#define zatm_free(ptr) free(ptr)
#define zatm_add(ptr, value) (*ptr += value)
#define zatm_sub(ptr, value) (*ptr -= value)
#define zatm_inc(ptr) (++(*ptr))
#define zatm_dec(ptr) (--(*ptr))
#define zatm_cas(ptr, oldval, newval) (*ptr == oldval) ? *ptr = newval : oldval;
#define zatm_xchg(ptr, newval) do{ptr ^= newval; newval ^= ptr; ptr ^= newval;}while(0)
#define zatm_xchg_ptr(ptr, newval) do{ptr ^= newval; newval ^= ptr; ptr ^= newval;}while(0)
#define zatm_barrier()

#endif

#if 0
// gcc 4.1.2 support __sync*
// Linux2.6.18
type __sync_fetch_and_add (type *ptr, type value);
type __sync_fetch_and_sub (type *ptr, type value);
type __sync_fetch_and_or (type *ptr, type value);
type __sync_fetch_and_and (type *ptr, type value);
type __sync_fetch_and_xor (type *ptr, type value);
type __sync_fetch_and_nand (type *ptr, type value);
type __sync_add_and_fetch (type *ptr, type value);
type __sync_sub_and_fetch (type *ptr, type value);
type __sync_or_and_fetch (type *ptr, type value);
type __sync_and_and_fetch (type *ptr, type value);
type __sync_xor_and_fetch (type *ptr, type value);
type __sync_nand_and_fetch (type *ptr, type value);

bool __sync_bool_compare_and_swap (type*ptr, type oldval, type newval, ...);
type __sync_val_compare_and_swap (type *ptr, type oldval,  type newval, ...);

__sync_synchronize (...);
//full barrier; http://blog.sunchangming.com/post/47188394133

type __sync_lock_test_and_set (type *ptr, type value, ...);
// set new val and return old val;

void __sync_lock_release (type *ptr, ...);//set *ptr = 0
#endif

#endif
