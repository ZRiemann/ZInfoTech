#ifndef _ZBASE_ATOMIC_H_
#define _ZBASE_ATOMIC_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
#ifdef ZSYS_POSIX
#include <zit/thread/mutex.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

typedef struct zatomic_t{
  volatile void* ptr;
  zmutex_t mtx;
}zatmc_t;

ZEXP zatmc_t* zatomic_create();
ZEXP void zatmic_destroy(zatmc_t** atm);
// exchange pointers, pointer is set 'v' and return old value. 
ZEXP void* zatomic_xchg(zatmic_t* atm, void* v);
// compare and swap, if 'cmp' equal then set 'v' and return old value.
ZEXP void* zatomic_cmpswap(zatmic_t* atm, void* cmp, void* v);

#ifdef __cplusplus
}
#endif
#endif
