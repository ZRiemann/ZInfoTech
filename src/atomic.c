#include "export.h"
#include <zit/base/trace.h>
#include <zit/base/atomic.h>
#include <stdlib.h>
zatmc_t* zatomic_create(){
  zatmc_t* atm = (zatmc_t*)malloc(sizeof(zatmc_t));
  if(NULL != atm){
    zmutex_init(&atm->mtx);
    zatomic_xchg(atm, NULL);
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
