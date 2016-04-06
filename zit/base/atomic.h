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

ZC_END

#endif
