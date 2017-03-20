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

ZC_END

#endif
