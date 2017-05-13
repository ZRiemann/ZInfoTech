#ifndef _ZBASE_ATOMIC_H_
#define _ZBASE_ATOMIC_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <zit/thread/mutex.h>

ZC_BEGIN

typedef struct zatomic_t{
  volatile void* ptr;
  volatile int value;
  zmtx_t mtx;
}zatmc_t;

ZAPI zatmc_t* zatomic_create();
ZAPI void zatomic_destroy(zatmc_t** atm);
ZAPI int zatomic_init(zatmc_t* atm);
ZAPI int zatomic_uninit(zatmc_t* atm);
// exchange pointers, pointer is set 'v' and return old value. 
ZAPI void* zatomic_xchg(zatmc_t* atm, void* v);
// compare and swap, if 'cmp' equal then set 'v' and return old value.
ZAPI void* zatomic_cmpswap(zatmc_t* atm, void* cmp, void* v);
// Increment value
// Decrement value
//==============================================================
// reimplement zit automic
#ifdef ZSYS_POSIX
#define ZATM_MUTEX // use mutex for hardware independent for linux
//#define ZATM_X86 // use x86 asm for linux
//#define ZATM_ARM // use ARM asm for linux
#else
#define ZATM_MUTEX // use mutex lock and unlock
#endif

ZAPI int ziatm_create(zatm_t *atm);
ZAPI int ziatm_destroy(zatm_t atm);
ZAPI zspin_t ziatm_xchg(zatm_t atm, zspin_t val);
ZAPI zspin_t ziatm_cas(zatm_t atm, zspin_t cmp, zspin_t val);
ZAPI zspin_t ziatm_inc(zatm_t atm);
ZAPI zspin_t ziatm_dec(zatm_t atm);
ZAPI zspin_t ziatm_add(zatm_t atm, zspin_t val);
ZAPI zspin_t ziatm_sub(zatm_t atm, zspin_t val);
ZAPI int ziatm_lock(zatm_t atm);
ZAPI int ziatm_unlock(zatm_t atm);

ZAPI int zpatm_create(zatm_t *atm);
ZAPI int zpatm_destroy(zatm_t atm);
ZAPI zptr_t zpatm_cas(zatm_t atm, zptr_t cmp, zptr_t ptr);
ZAPI zptr_t zpatm_xchg(zatm_t atm, zptr_t ptr);

//=============================================================
// 3rd edition

#ifdef ZSYS_POSIX
#define zatm_add(ptr, value) __sync_add_and_fetch((ptr), (value))
#define zatm_sub(ptr, value) __sync_sub_and_fetch((ptr), (value))
#define zatm_cas(ptr, oldval, newval) __sync_val_compare_and_swap(type *ptr,oldval,newval)
#endif

#if 0
// gcc从4.1.2提供了__sync_*系列的built-in函数，用于提供加减和逻辑运算的原子操作
// 在Linux2.6.18之后,GCC提供了内置的原子操作函数，更适合用户态的程序使用。
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
//理解上面这个东西，参照：http://blog.sunchangming.com/post/47188394133

type __sync_lock_test_and_set (type *ptr, type value, ...);
//将*ptr设为value并返回*ptr操作之前的值。

void __sync_lock_release (type *ptr, ...);//将*ptr置0
#endif

ZC_END

#endif
