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

ZAPI zspin_t ziatm_xchg(zatm_t atm, zspin_t val){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedExchange(&iatm->spin, val);
#elif (defined ZATM_MUTEX || defined ZATM_X86 || defined ZATM_ARM)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = iatm->spin;
  iatm->spin = val;
  ZUNLOCK(&iatm->mtx);
  return old;
#else
#error atomic is not implemented for this platform  
#endif
}
zspin_t ziatm_cas(zatm_t atm, zspin_t cmp, zspin_t val){
#ifdef ZSYS_WINDOWS
  return (val == (zspin_t)InterlockedCompareExchange(&iatm->spin, val, cmp));
#elif (defined ZATM_MUTEX || defined ZATM_X86 || defined ZATM_ARM)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = iatm->spin;
  if(cmp == old){
    iatm->spin = val;
  }
  ZUNLOCK(&iatm->mtx);
  return (old == val);
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
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  __asm__ volatile (
		    "lock; xadd %0, %1 \n\t"
		    : "=r" (old), "=m" (*val)
		    : "0" (inc), "m" (*val)
		    : "cc", "memory");
  return *val;
#elif (defined ZATM_ARM)
  zspin_t flag, tmp;
  zspin_t old,inc;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  inc = 1;
  old = 0;
  __asm__ volatile (
		    "       dmb     sy\n\t"
		    "1:     ldrex   %0, [%5]\n\t"
		    "       add     %2, %0, %4\n\t"
		    "       strex   %1, %2, [%5]\n\t"
		    "       teq     %1, #0\n\t"
		    "       bne     1b\n\t"
		    "       dmb     sy\n\t"
		    : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(*val)
		    : "Ir"(inc), "r"(val)
		    : "cc");
  return *val;
#else
#error atomic is not implemented for this platform  
#endif
  return(ZOK);
}

zspin_t ziatm_dec(zatm_t atm){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedDecrement(&iatm->spin);
#elif (defined ZATM_MUTEX)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = --iatm->spin;
  ZUNLOCK(&iatm->mtx);
  return old;
#elif (defined ZATM_X86)
  zspin_t inc = -1;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  __asm__ volatile (
		    "lock; xaddl %0, %1 \n\t"
		    : "=r" (inc), "=m" (*val)
		    : "0" (inc), "m" (*val)
		    : "cc", "memory");
  return *val;
#elif (defined ZATM_ARM)
  zspin_t flag, tmp;
  zspin_t old,dec;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  dec = 1;
  old = 0;
  __asm__ volatile (
		    "       dmb     sy\n\t"
		    "1:     ldrex   %0, [%5]\n\t"
		    "       sub     %2, %0, %4\n\t"
		    "       strex   %1, %2, [%5]\n\t"
		    "       teq     %1, #0\n\t"
		    "       bne     1b\n\t"
		    "       dmb     sy\n\t"
		    : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(*val)
		    : "Ir"(dec), "r"(val)
		    : "cc");
  return *val;
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}

zspin_t ziatm_add(zatm_t atm, zspin_t inc){
#ifdef ZSYS_WINDOWS
  return (zpin_t)InterlockedExchangeAdd (&iatm->spin, val);
#elif (defined ZATM_MUTEX)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = iatm->spin;
  iatm->spin += inc;
  ZUNLOCK(&iatm->mtx);
  return old;
#elif (defined ZATM_X86)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  __asm__ volatile (
		    "lock; xadd %0, %1 \n\t"
		    : "=r" (old), "=m" (*val)
		    : "0" (inc), "m" (*val)
		    : "cc", "memory");
  return old;
#elif (defined ZATM_ARM)
  zspin_t flag, tmp;
  zspin_t old;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  old = 0;
  __asm__ volatile (
		    "       dmb     sy\n\t"
		    "1:     ldrex   %0, [%5]\n\t"
		    "       add     %2, %0, %4\n\t"
		    "       strex   %1, %2, [%5]\n\t"
		    "       teq     %1, #0\n\t"
		    "       bne     1b\n\t"
		    "       dmb     sy\n\t"
		    : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(*val)
		    : "Ir"(inc), "r"(val)
		    : "cc");
  return old;
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}
zspin_t ziatm_sub(zatm_t atm, zspin_t dec){
#ifdef ZSYS_WINDOWS
  return (zspin_t)InterlockedExchangeAdd (&iatm->spin, -val);
#elif (defined ZATM_MUTEX)
  zspin_t old = 0;
  ziatm_t *iatm = (ziatm_t*)atm;
  ZLOCK(&iatm->mtx);
  old = iatm->spin;
  iatm->spin -= dec;
  ZUNLOCK(&iatm->mtx);
  return old;
#elif (defined ZATM_X86)
  zspin_t inc = -dec;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  __asm__ volatile (
		    "lock; xaddl %0, %1 \n\t"
		    : "=r" (inc), "=m" (*val)
		    : "0" (inc), "m" (*val)
		    : "cc", "memory");
  return *val;
#elif (defined ZATM_ARM)
  zspin_t flag, tmp;
  zspin_t old;
  ziatm_t *iatm = (ziatm_t*)atm;
  volatile zspin_t *val = &iatm->spin;
  old = 0;
  __asm__ volatile (
		    "       dmb     sy\n\t"
		    "1:     ldrex   %0, [%5]\n\t"
		    "       sub     %2, %0, %4\n\t"
		    "       strex   %1, %2, [%5]\n\t"
		    "       teq     %1, #0\n\t"
		    "       bne     1b\n\t"
		    "       dmb     sy\n\t"
		    : "=&r"(old), "=&r"(flag), "=&r"(tmp), "+Qo"(*val)
		    : "Ir"(dec), "r"(val)
		    : "cc");
  return old;
#else
#error atomic is not implemented for this platform  
#endif

  return(ZOK);
}

int ziatm_lock(zatm_t atm){
  return zmutex_lock(&((ziatm_t*)atm)->mtx);
}
int ziatm_unlock(zatm_t atm){
  return zmutex_unlock(&((ziatm_t*)atm)->mtx);
}

//=================================================
// zpatm_t
typedef struct zpatm_s{
  volatile zptr_t ptr;
#ifdef ZSYS_POSIX //ZATM_MUTEX
  zmtx_t mtx;
#endif
}zpatm_t;

int zpatm_create(zatm_t *atm){
  int ret = ZOK;
  zpatm_t *patm;
  ZASSERT(!atm);
  *atm = malloc(sizeof(ziatm_t));
  if(!*atm)return(ZMEM_INSUFFICIENT);
  patm = (zpatm_t*)*atm;
  patm->ptr = NULL;
#ifdef ZSYS_POSIX //ZATM_MUTEX
  ret = zmutex_init(&patm->mtx);
  ZERRCX(ret);
#endif
  return(ret);
}
int zpatm_destroy(zatm_t atm){
  zpatm_t *patm = (zpatm_t*)atm;
#ifdef ZSYS_POSIX //ZATM_MUTEX
  if(patm)zmutex_uninit(&patm->mtx);
#endif
  free(atm);
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
  return (zptr_t)InterlockedExchangePointer(&patm->ptr, ptr);
#elif (defined ZATM_MUTEX)
  zptr_t p;
  zpatm_t *patm = (zpatm_t*)atm;
  ZLOCK(&patm->mtx);
  p = patm->ptr;
  patm->ptr = ptr;
  ZUNLOCK(&patm->mtx);
  return p;
#elif (defined ZATM_X86)
  zptr_t old;
  zpatm_t *patm = (zpatm_t*)atm;
  volatile zptr_t *org = &patm->ptr;
  __asm__ volatile (
     "lock; xchg %0, %2"
     : "=r" (old), "=m" (*org)
     : "m" (*org), "0" (ptr));
  return old;
#elif (defined ZATM_ARM)
  zptr_t old;
  unsigned int flag;
  zpatm_t *patm = (zpatm_t*)atm;
  volatile zptr_t *org = &patm->ptr;
  __asm__ volatile (
    "       dmb     sy\n\t"
    "1:     ldrex   %1, [%3]\n\t"
    "       strex   %0, %4, [%3]\n\t"
    "       teq     %0, #0\n\t"
    "       bne     1b\n\t"
    "       dmb     sy\n\t"
    : "=&r"(flag), "=&r"(old), "+Qo"(*org)
    : "r"(org), "r"(ptr)
    : "cc");
    return old;
#else
#error atomic is not implemented for this platform  
#endif

  return(NULL);
}
