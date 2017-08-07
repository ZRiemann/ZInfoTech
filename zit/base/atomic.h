#ifndef _ZBASE_ATOMIC_H_
#define _ZBASE_ATOMIC_H_

#include <zit/base/platform.h>

#ifdef ZSYS_WINDOWS
#include "inttypes.h"
#elif defined(ZSYS_POSIX)
#include <inttypes.h>
#endif//ZSYS_WINDOWS

typedef int zatmi_t;
typedef uint32_t zatm32_t;
typedef uint64_t zatm64_t;
typedef void* zatmp_t;

#ifdef ZSYS_POSIX
// gcc
#define zatm_alloc(size) memalign(64, size)
#define zatm_free(ptr) free(ptr)
#define zatm_add(ptr, value) __sync_add_and_fetch(ptr, value)
#define zatm_sub(ptr, value) __sync_sub_and_fetch(ptr, value)
#define zatm_inc(ptr) __sync_add_and_fetch(ptr, 1)
#define zatm_dec(ptr) __sync_sub_and_fetch(ptr, 1)
#define zatm_cas(ptr, oldval, newval) __sync_val_compare_and_swap(type *ptr,oldval,newval)
#define zatm_xchg(ptr, newval) __sync_lock_test_and_set (ptr, newval);
#define zatm_xchg_ptr(ptr, newval) __sync_lock_test_and_set (ptr, newval);

#elif defined(ZSYS_WINDOWS)

#include <Windows.h>
#define zatm_alloc(size) _aligned_malloc(size, 64)
#define zatm_free(ptr) free(ptr)
#define zatm_add(ptr, value) (sizeof(*ptr)==64 ? InterlockedExchangeAdd64(ptr, value) : InterlockedExchangeAdd(ptr, value))
#define zatm_sub(ptr, value) (sizeof(*ptr)==64 ? InterlockedExchangeAdd(ptr, -(value)) : InterlockedExchangeAdd(ptr, -(value)))
#define zatm_inc(ptr) (sizeof(*ptr)==64 ? InterlockedIncrement64(ptr) : InterlockedIncrement(ptr))
#define zatm_dec(ptr) (sizeof(*ptr)==64 ? InterlockedDecrement64(ptr) : InterlockedDecrement(ptr))
#define zatm_cas(ptr, oldval, newval) (sizeof(*ptr)==64 ? InterlockedCompareExchange64(ptr, newval, oldval) : InterlockedCompareExchange(ptr, newval, oldval))
#define zatm_xchg(ptr, newval) (sizeof(*ptr)==64 ? InterlockedExchange64(ptr, newval) : InterlockedExchange(ptr, newval))
#define zatm_xchg_ptr(ptr, newval) InterlockedExchangePointer(ptr, newval)

#else // some embend code, no threads

#define zatm_alloc(size)
#define zatm_free(ptr)
#define zatm_add(ptr, value)
#define zatm_sub(ptr, value)
#define zatm_inc(ptr)
#define zatm_dec(ptr)
#define zatm_cas(ptr, oldval, newval)
#define zatm_xchg(ptr, newval)
#define zatm_xchg_ptr(ptr, newval)

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
