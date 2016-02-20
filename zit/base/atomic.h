#ifndef _ZBASE_ATOMIC_H_
#define _ZBASE_ATOMIC_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <zit/thread/mutex.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct zatomic_t{
  volatile void* ptr;
  zmtx_t mtx;
}zatmc_t;

ZEXP zatmc_t* zatomic_create();
ZEXP void zatomic_destroy(zatmc_t** atm);
ZEXP int zatomic_init(zatmc_t* atm);
ZEXP int zatomic_uninit(zatmc_t* atm);
// exchange pointers, pointer is set 'v' and return old value. 
ZEXP void* zatomic_xchg(zatmc_t* atm, void* v);
// compare and swap, if 'cmp' equal then set 'v' and return old value.
ZEXP void* zatomic_cmpswap(zatmc_t* atm, void* cmp, void* v);

#ifdef __cplusplus
}
#endif
#endif
