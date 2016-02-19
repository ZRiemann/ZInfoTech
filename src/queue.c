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
  que->chnkback->v[que->posback] = value;
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
      ZDBG("exchange sqare chunk.");
    }
  }
  ZERRCX(ret);
  return ret;
}

int zqueue_pushfront(zque_t* que, zvalue_t value){
  int ret = ZEOK;
  if(--(que->posfront) == 0){
    zchnk_t* chnksqare = zatomic_xchg(que->atm, NULL);
    if((NULL == chnksqare) && (NULL ==(chnksqare = (zchnk_t*)malloc(sizeof(zchnk_t))))){
      ++(que->posback);
      ret = ZEMEM_INSUFFICIENT;
    }else{
      que->chnkfront->prev = chnksqare;
      chnksqare->next = que->chnkfront;
      que->chnkfront = chnksqare;
      que->posfront = ZQUEUE_SIZE-1;
      que->chnkfront->v[que->posfront] = value;
      ZDBG("exchange sqare chunk.");
    }
  }else{
    que->chnkfront->v[que->posfront] = value;
  }
  ZERRCX(ret);
  return ret;
}

int zqueue_popback(zque_t* que, zvalue_t* value){
  int ret = ZEOK;
  if(que->chnkfront == que->chnkback){
    if(que->posfornt < que->posback){
      --(que->posback);
    }else{
      ret = ZENOT_EXIST;
    }
  }else if(--(que->posback) < 0){
    zchnk_t* chnkback = que->chnkback;
    zchnk_t* chnksqare = NULL;
    que->posback = ZQUEUE_SIZE-1;
    que->chnkback = que->chnkback->prev;
    que->chnkback->next = NULL;
    chnksqare = (zchnk_t*)zatomic_xchg(chnkback);
    free(chnksqare);
    ZDBG("exchange sqare chunk");
  }
  *value = que->chnkback->v[que->posback];
  //ZERRCX(ret);
  return ret;
}

int zqueuq_popfornt(zque_t* que, zvalue_t* value){
  int ret = ZEOK;
  *value = que->chnkfront->v[que->posfront];
  if(que->chnkfront == que->chnkback){
    if(que->posfront < que->posback){
      ++(que->posfront);
    }else{
      ret = ZENOT_EXIST;
    }
  }else if(++(que->posfront) == ZQUEUE_SIZE){
    zchnk_t* chnkfront = que->chnkfront;
    zchnk_t* chnksqare = NULL;
    que->posfront = 0;
    que->chnkfront = que->chnkfornt->next;
    que->chnkfront->prev = NULL;
    chnksqare = zatomic_xchg(chnkfront);
    free(chnksqare);
    ZDBG("exchange sqare chunk");
  }
  //ZERRCX(ret);
  return ret;
}

int zqueue_foreach(zque_t* que, ztsk_t* tsk){
  int ret = ZEOK;
  zchnk_t* visitor = que->chnkfront;
  int posend = 0;
  int posbegin = que->posfront;
  int end = 0;
  while(1){
    if(visitor == que->chnkback){
      posend = que->posback;
      end = 1;
    }else{
      posend = ZQUEUE_SIZE;
      visitor = visitor->next;
    }
    
    while(posbegin < posend){
      tsk->act(visitor->v[posbegin], tsk->hint);
      ++posbegin;
    }
    posbegin = 0;
    if(1 == end)break;
  }
  //ZERRC(ret);
  return ret;
}
