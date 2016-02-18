#include "export.h"
#include <zit/base/trace.h>
#include <zit/base/error.h>
#include <zit/base/queue.h>
#include <stdlib.h>

/*
atm->ptr -->NULL/spare_chunk
NULL<--chnkfront<-->...<-->chnkback-->NULL

# create
atm->ptr -->NULL
chnkfront 0 1 2 3 ... ZQUEUE_SIZE-1
          ^
	  posfront/posback

# push back (v)
1. normal condition
atm->ptr -->NULL/spare_chunk
posback'--| |--posback
 chnkback 0 1 2 3 ... ZQUEUE_SIZE-1
          |--posfront(v)

2. critical condition
before push back:
atm->ptr -->NULL/spare_chunk
chnkback 0 1 2 3 ... ZQUEUE_SIZE-1
                     |--posback
after push back:
atm->ptr -->NULL
(old_chnkback) 0 1 2 3 ... ZQUEUE_SIZE-1 -->(new_chnkback/spare_chunk)
                           |-- (v)
(old_chnkback)<--chnkback(new_chnkback/spare_chunk) 0 1 2 3 ... ZQUEUE_SIZE-1
                                                    |--posback

# pop back (v)
1. normal condition
atm->ptr -->NULL/spare_chunk
chnkback 0 1 2 3 ... ZQUEUE_SIZE-1
    posback--| |--posback'
2. critical condition
before pop back:
atm->ptr -->NULL/spare_chunk
pre_chnkback<--old_chnkback 0 1 2 3 ... ZQUEUE_SIZE-1
                            |--posback
after pop back:
atm->ptr -->(old cnhkback)/(old chnkback and free spare_chunk)
pre_chnkback 0 1 2 3 ... ZQUEUE_SIZE-1
                                |--posback

# push/pop front similer push/pop back.
*/
int zqueue_create(zque_t** que){
  int ret = ZEOK;
  zque_t* pque = (zque_t*)malloc(sizeof(zque_t));
  zatmc_t* atm = (zatmc_t*)malloc(sizeof(zatmc_t));
  zchnk_t* chnk = (zchnk_t*)malloc(sizeof(zchnk_t));

  if((NULL == pque) || (NULL == atm) || (NULL == chnk)){
    *que = NULL;
    free(pque);
    free(atm);
    free(chnk);
    ret = ZEMEM_INSUFFICIENT;
  }else{
    *que = pque;
    pque->atm = atm;
    pque->chnkfront = pque->chnkback = chnk;
    pque->posfront = pque->posback = 0;
    chnk->prev = chnk->next = NULL;
  }
  ZERRC(ret);
  return ret;
}

int zqueue_destroy(zque_t** que){
  int ret = ZEOK;
  if(NULL != *que){
    free(zatomic_xchg((*que)->atm, NULL))
    zatomic_destroy((*que)->atm);
    free(*que);
    *que = NULL;
  }
  ZERRC(ret);
  return ret;
}

int zqueue_pushback(zque_t* que, zvalue_t value){
  int ret = ZEOK;
  que->chnkback[que->posback] = value;
  if(++(que->posback) == ZQUEUE_SIZE){
    zchnk_t* chnksqare = zatomic_xchg(que->atm, NULL);
    if((NULL == chnksqare) && (NULL ==(chnksqare = (zchnk_t*)malloc(sizeof(zchnk_t))))){
      --(que->posback);
      ret = ZEMEM_INSUFFICIENT;
    }else{
      que->chnkback->next = chnksqare;
      chnksqare->prev = que->chnkback;
      que->chnkback = chnksqre;
      que->posback = 0;
    }
  }
  ZERRCX(ret);
  return ret;
}

int zqueue_pushfront(zque_t* que, zvalue_t value){
  int ret = ZEOK;
  que->chnkfront
  ZERRCX(ret);
  return ret;
}

int zqueue_popback(zque_t* que, zvalue_t* value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueuq_popfornt(zque_t* que, zvalue_t* value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_foreach(zque_t* que, ztsk_t* tsk){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}
