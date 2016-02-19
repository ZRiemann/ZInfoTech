#include "export.h"
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
void zatmic_destroy(zatmc_t** atm){
  if((NULL != atm) && (NULL != *atm)){
    zmutex_uninit(&((*atm)->mtx));
    free(*atm);
    *atm = NULL;
  }
}
void* zatomic_xchg(zatmc_t* atm, void* v){
#ifdef ZSYS_WINDOWS
  return (void*) InterlockedExchangePointer ((PVOID*) &ptr, v);
#else
  void* old = NULL;
  zmutex_lock(&atm->mtx);
  old = (void*)ptr;
  ptr = v;
  zmutex_unlock(&atm->mtx);
  return old;
#endif
}

void* zatomic_cmpswap(zatmc_t* atm, void* cmp, void* v){
#ifdef ZSYS_WINDOWS
  return (void*)InterlockedCompareExchangePointer((volatile PVOID*)&ptr, v, cmp);
#else
  void* old = NULL;
  zmutex_lock(&atm->mtx);
  old = (void*)ptr;
  if(ptr == cmp){
    ptr = v;
  }
  zmutex_unlock(&atm->mtx);
  return old;
#endif
}
