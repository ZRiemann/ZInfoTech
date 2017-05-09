#include "export.h"
#include <zit/base/trace.h>
#include <zit/base/error.h>
#include <zit/base/queue.h>
#include <zit/base/atomic.h>
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

#define ZQUEUE_SIZE 4090
typedef struct zchunk_t{
  zvalue_t v[ZQUEUE_SIZE];
  struct zchunk_t* next;
  struct zchunk_t* prev;
}zchnk_t;

typedef struct zqueue_t{
  zchnk_t* chnkback;
  zchnk_t* chnkfront;
  zatm_t atm_spare;
  zatm_t atm_lock;
  int posfront;
  int posback;
  zsize_t size;
}zque_t;

int zque_create(zcontainer_t *cont){
  int ret = ZOK;
  zchnk_t* chnk = (zchnk_t*)malloc(sizeof(zchnk_t));
  zque_t* pque = (zque_t*)malloc(sizeof(zque_t));

  if((NULL == pque) || (NULL == chnk)){
    *cont = NULL;
    free(pque);
    free(chnk);
    ret = ZMEM_INSUFFICIENT;
  }else{
    *cont = (zcontainer_t)pque;
    zpatm_create(&pque->atm_spare);
    ziatm_create(&pque->atm_lock);
    pque->chnkfront = pque->chnkback = chnk;
    pque->posfront = pque->posback = 0;
    chnk->prev = chnk->next = NULL;
    pque->size = 0;
  }
#if ZTRACE_QUE
  ZERRC(ret);
#endif
  return(ret);
}

int zque_destroy(zcontainer_t cont, zoperate release){
  zque_t *que = (zque_t*)cont;
  zchnk_t* chnkfront;
  void* ptr;

  if(NULL != que){
    // release data
    if(release)zque_foreach(cont, release, NULL);
    while(que->chnkfront){
      chnkfront = que->chnkfront;
      que->chnkfront = que->chnkfront->next;
      //zdbg("free queue chunk<%p>", chnkfront);
      free(chnkfront);
    }
    
    ptr = zpatm_xchg(que->atm_spare, (void*)NULL);
    free(ptr);
    // destroy
    zpatm_destroy(que->atm_spare);
    ziatm_destroy(que->atm_lock);
    free(que);
  }
#if ZTRACE_QUE
  ZERRC(ZOK);
#endif
  return(ZOK);
}

zsize_t zque_size(zcontainer_t cont){
  return ((zque_t*)cont)->size;
}

int zque_push(zcontainer_t cont, zvalue_t in){
  zque_t *que;
  int ret;

  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){ZERRC(ret);return ret;}
  ++que->size;
  que->chnkback->v[que->posback] = in;
  if(++(que->posback) == ZQUEUE_SIZE){
    zchnk_t* chnksqare = zpatm_xchg(que->atm_spare, NULL);
    if((NULL == chnksqare) && (NULL ==(chnksqare = (zchnk_t*)malloc(sizeof(zchnk_t))))){
      --(que->posback);
      --que->size;
      ret = ZEMEM_INSUFFICIENT;
    }else{
      que->chnkback->next = chnksqare;
      chnksqare->prev = que->chnkback;
      chnksqare->next = NULL;
      que->chnkback = chnksqare;
      que->posback = 0;
      //  ZDBG("push exchange sqare chunk.");
    }
  }
  ziatm_unlock(que->atm_lock);
  return(ret);
}

int zque_pop(zcontainer_t cont, zvalue_t *out){
  zque_t *que;
  int ret = ZOK;
  //ZASSERT(!cont || !out)
  que  = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock);if(ZOK != ret){return ret;}
  *out = que->chnkfront->v[que->posfront];
  --que->size;
  if(que->chnkfront == que->chnkback){
    if(que->posfront < que->posback){
      ++(que->posfront);// reset position ** disable for push pop not thread save
    }else{
      ++que->size;
      *out = NULL;
      ret = ZNOT_EXIST;
    }
  }else if(++(que->posfront) == ZQUEUE_SIZE){
    zchnk_t* chnkfront = que->chnkfront;
    zchnk_t* chnksqare = NULL;
    que->posfront = 0;
    que->chnkfront = que->chnkfront->next;
    que->chnkfront->prev = NULL;
    chnksqare = zpatm_xchg(que->atm_spare, chnkfront);
    free(chnksqare);
    //    ZDBG("pop exchange sqare chunk");
  }
  //ZERRCX(ret);
  //if(ZOK == ret)--que->size;
  ziatm_unlock(que->atm_lock);
  return(ret);
}

int zque_pushfront(zcontainer_t cont, zvalue_t in){
  zque_t *que;
  int ret = ZEOK;
  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret;}
  ++que->size;
  if(--(que->posfront) < 0){
    zchnk_t* chnksqare = zpatm_xchg(que->atm_spare, NULL);
    if((NULL == chnksqare) && (NULL ==(chnksqare = (zchnk_t*)malloc(sizeof(zchnk_t))))){
      ++(que->posfront);
      ret = ZEMEM_INSUFFICIENT;
      --que->size;
    }else{
      que->chnkfront->prev = chnksqare;
      chnksqare->next = que->chnkfront;
      chnksqare->prev = NULL;
      que->chnkfront = chnksqare;
      que->posfront = ZQUEUE_SIZE-1;
      que->chnkfront->v[que->posfront] = in;
      //ZDBG("push exchange sqare chunk.");
    }
  }else{
    que->chnkfront->v[que->posfront] = in;
  }
  //ZERRCX(ret);
  ziatm_unlock(que->atm_lock);
  return ret;

}

int zque_popback(zcontainer_t cont, zvalue_t *out){
  zque_t *que;
  int ret = ZOK;
  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret;}
  --que->size;
  if(que->chnkfront == que->chnkback){
    if(que->posfront < que->posback){
      --(que->posback);
    }else{
      ++que->size;
      *out = NULL;
      ret = ZNOT_EXIST;
    }
  }else if(--(que->posback) < 0){
    zchnk_t* chnkback = que->chnkback;
    zchnk_t* chnksqare = NULL;
    que->posback = ZQUEUE_SIZE-1;
    que->chnkback = que->chnkback->prev;
    que->chnkback->next = NULL;
    chnksqare = (zchnk_t*)zpatm_xchg(que->atm_spare, chnkback);
    free(chnksqare);
    //ZDBG("pop exchange sqare chunk");
  }
  *out = que->chnkback->v[que->posback];
  //ZERRCX(ret);
  ziatm_unlock(que->atm_lock);
  return ret;
}
int zque_insert(zcontainer_t cont, zvalue_t in, zoperate compare){
  //  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret}
  //  ziatm_unlock(que->atm_lock);
  return(ZNOT_SUPPORT);
}
int zque_erase(zcontainer_t cont, zvalue_t in, zoperate compare){
  //  ret = ziatm_lock(atm_lock); if(ZOK != ret){return ret}
  //  ziatm_unlock(atm_lock);
  return(ZNOT_SUPPORT);
}

int zque_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
  zque_t *que;
  int ret;
  zchnk_t* visitor;
  int posend;
  int posbegin;
  int end;
  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret;}
  que = (zque_t*)cont;
  posend = end = 0;
  visitor = que->chnkfront;
  posbegin = que->posfront;
  while(1){
    if(visitor == que->chnkback){
      posend = que->posback;
      end = 1;
    }else{
      posend = ZQUEUE_SIZE;
    }
    
    while(posbegin < posend){
      ret = op(visitor->v[posbegin], NULL, hint);
      if(ZCMD_STOP == ret){
	break;
      }
      ++posbegin;
    }
    
    if(1 == end){
      break;
    }else{
      posbegin = 0;
      visitor = visitor->next;
    }
  }
  //ZERRC(ret);
  ziatm_unlock(que->atm_lock);
  return(ret);
}

int zque_back(zcontainer_t cont, zvalue_t *out){
  zque_t *que;
  int ret = ZOK;
  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret;}
  if(que->size>0){
    *out = que->chnkback->v[que->posback];
  }else{
    ret = ZNOT_EXIST;
  }
  //ZERRCX(ret);
  ziatm_unlock(que->atm_lock);
  return ret;
}
int zque_front(zcontainer_t cont, zvalue_t *out){
  zque_t *que;
  int ret = ZOK;
  que = (zque_t*)cont;
  ret = ziatm_lock(que->atm_lock); if(ZOK != ret){return ret;}
  if(que->size>0){
    *out = que->chnkfront->v[que->posfront];
  }else{
    ret = ZNOT_EXIST;
  }
  //ZERRCX(ret);
  ziatm_unlock(que->atm_lock);
  return ret;
}

int zque_swap(zcontainer_t *cont1, zcontainer_t *cont2){
  int ret;
  zque_t *que1;
  zque_t *que2;

  que1 = (zque_t*)(*cont1);
  que2 = (zque_t*)(*cont2);

  ret = ziatm_lock(que1->atm_lock); if(ZOK != ret){return ret;}
  ret = ziatm_lock(que2->atm_lock); if(ZOK != ret){
    ziatm_unlock(que1->atm_lock);    
    return ret;
  }

  *cont1 = que2;
  *cont2 = que1;
  
  ziatm_unlock(que2->atm_lock);
  ziatm_unlock(que1->atm_lock);
  return ZOK;
}
