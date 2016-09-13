#include "export.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/atomic.h>
#include <stdlib.h>


#ifdef ZATM_MUTEX
#include <zit/thread/mutex.h>
#endif

zatmc_t* zatomic_create(){
  zatmc_t* atm = (zatmc_t*)malloc(sizeof(zatmc_t));
  if(NULL != atm){
    zmutex_init(&atm->mtx);
    zatomic_xchg(atm, NULL);
    atm->value = 0;
  }
  return atm;
}
void zatomic_destroy(zatmc_t** atm){
  if((NULL != atm) && (NULL != *atm)){
    zmutex_uninit(&((*atm)->mtx));
    free(*atm);
    *atm = NULL;
  }
}

int zatomic_init(zatmc_t* atm){
  int ret = ZEOK;
  if(NULL != atm){
    ret = zmutex_init(&atm->mtx);
    zatomic_xchg(atm, NULL);
    atm->value = 0;
  }
  ZERRC(ret);
  return ret;
}

int zatomic_uninit(zatmc_t* atm){
  int ret = ZEOK;
  if(NULL != atm){
    ret = zmutex_uninit(&(atm->mtx));
  }
  ZERRC(ret);
  return ret;
}

void* zatomic_xchg(zatmc_t* atm, void* v){
#ifdef ZSYS_WINDOWS
  return (void*) InterlockedExchangePointer ((PVOID*)&(atm->ptr), v);
#else
  void* old = NULL;
  zmutex_lock(&atm->mtx);
  old = (void*)atm->ptr;
  atm->ptr = v;
  zmutex_unlock(&atm->mtx);
  return old;
#endif
}

void* zatomic_cmpswap(zatmc_t* atm, void* cmp, void* v){
#ifdef ZSYS_WINDOWS
  return (void*)InterlockedCompareExchangePointer((volatile PVOID*)&(atm->ptr), v, cmp);
#else
  void* old = NULL;
  zmutex_lock(&atm->mtx);
  old = (void*)atm->ptr;
  if(atm->ptr == cmp){
    atm->ptr = v;
  }
  zmutex_unlock(&atm->mtx);
  return old;
#endif
}


//===================================================
// reimplement
typedef struct ziatm_s{
  volatile zspin_t spin;
#ifdef ZSYS_POSIX//ZATM_MUTEX
  zmtx_t mtx;
#endif
}ziatm_t;

int ziatm_create(zatm_t *atm){
  int ret = ZOK;
  ziatm_t *iatm;
  ZASSERT(!atm);
  *atm = malloc(sizeof(ziatm_t));
  if(!*atm)return(ZMEM_INSUFFICIENT);
  iatm = (ziatm_t*)*atm;
  iatm->spin = 0;
#ifdef ZSYS_POSIX //ZATM_MUTEX
  ret = zmutex_init(&iatm->mtx);
  ZERRCX(ret);
#endif
  return(ret);
}

int ziatm_destroy(zatm_t atm){
  ziatm_t *iatm = (ziatm_t*)atm;
#ifdef ZSYS_POSIX //ZATM_MUTEX
  if(iatm)zmutex_uninit(&iatm->mtx);
#endif
  free(atm);
  return(ZOK);
}

zspin_t ziatm_cas(zatm_t atm, zspin_t cmp, zspin_t val){
  ziatm_t *iatm = (ziatm_t*)atm;
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedCompareExchange(&iatm->spin, val, cmp);
#elif (defined ZATM_MUTEX || defined ZATM_X86 || ZATM_ARM)
  zspin_t old = 0;
  ZLOCK(&iatm->mtx);
  old = iatm->spin;
  if(cmp == old){
    iatm->spin = val;
  }
  ZUNLOCK(&iatm->mtx);
  return old;
  //#elif (defined ZATM_X86)
    //#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif
  return(ZOK);
}

zspin_t ziatm_inc(zatm_t atm){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedIncrement(&iatm->spin);
#elif (defined ZATM_MUTEX)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = ++iatm->spin;
  ZUNLOCK(&iatm->mtx);
  return old;
#elif (defined ZATM_X86)
  zspin_t old = 0;
  zspin_t inc = 1;
  volatile integer_t *val = &iatm->spin;
  __asm__ volatile (
		    "lock; xadd %0, %1 \n\t"
		    : "=r" (old), "=m" (*val)
		    : "0" (inc), "m" (*val)
		    : "cc", "memory");
#elif (defined ZATM_ARM)
  integer_t flag, tmp;
  __asm__ volatile (
		    "       dmb     sy\n\t"
		    "1:     ldrex   %0, [%5]\n\t"
		    "       add     %2, %0, %4\n\t"
		    "       strex   %1, %2, [%5]\n\t"
		    "       teq     %1, #0\n\t"
		    "       bne     1b\n\t"
		    "       dmb     sy\n\t"
		    : "=&r"(old_value), "=&r"(flag), "=&r"(tmp), "+Qo"(value)
		    : "Ir"(increment_), "r"(&value)
		    : "cc");
#else
#error atomic is not implemented for this platform  
#endif
  return(ZOK);
}

zspin_t ziatm_dec(zatm_t atm){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedDecrement(&iatm->spin);
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}
zspin_t ziatm_add(zatm_t atm, zspin_t val){
#ifdef ZSYS_WINDOWS
  return (zpin_t)InterlockedExchangeAdd (&iatm->spin, val);
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}
zspin_t ziatm_sub(zatm_t atm, zspin_t val){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedExchangeAdd (&iatm->spin, -val);
#elif (defined ZATM_MUTEX)
  
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}


typedef struct zpatm_s{
  volatile zptr_t ptr;
#ifdef ZATM_MUTEX
  zmtx_t mtx;
#endif
}zpatm_t;

int zpatm_create(zatm_t *atm){
#ifdef ZSYS_WINDOWS
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}
int zpatm_destroy(zatm_t atm){
#ifdef ZSYS_WINDOWS
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}
zptr_t zpatm_cas(zatm_t atm, zptr_t cmp, zptr_t ptr){
#ifdef ZSYS_WINDOWS
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(NULL);
}
zptr_t zpatm_xchg(zatm_t atm, zptr_t ptr){
#ifdef ZSYS_WINDOWS
#elif (defined ZATM_MUTEX)
#elif (defined ZATM_X86)
#elif (defined ZATM_ARM)
#else
#error atomic is not implemented for this platform  
#endif

  return(NULL);
}
