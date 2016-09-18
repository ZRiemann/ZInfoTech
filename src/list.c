/**@file zit/src/list.c
 * @brief general list implement
 * @note
 */
#include "export.h"
#include <zit/base/list.h>
#include <zit/base/error.h>
#include <zit/base/atomic.h>
#include <stdlib.h>

/*
              ptail                      phead
  NULL(next)<-tail<->...<->nod2<->nod1<->head->NULL(prev)
*/
typedef struct zlist_s{
  znod_t *head;
  znod_t *tail;
  znod_t *recycle;
  uint32_t cnt_recycle;
  uint32_t max_recycle;
  size_t num;  // list size
  zatm_t atm;
}zlist_t;

int zlist_create(zcontainer_t *cont){
  zlist_t *list;
  
  *cont = malloc(sizeof(zlist_t));
  ZASSERT(!(*cont));
  list = (zlist_t*)(*cont);
  list->head = list->tail = list->recycle = NULL;
  list->max_recycle = 1024;
  list->cnt_recycle = 0;
  list->num = 0;
  ziatm_create(&list->atm);
  return(ZOK);
}

int zlist_destroy(zcontainer_t cont, zoperate release){
  znod_t *nod;
  zlist_t *list;

  ZASSERT(!cont);
  list = (zlist_t*)cont;
  while(list->recycle){
    nod = list->recycle;
    list->recycle = list->recycle->next;
    free(nod);
  }
  while(list->tail){
    nod = list->tail;
    list->tail = list->tail->prev;
    if(release){
      release(OPIN(nod->value));
    }
    free(nod);
  }
  ziatm_destroy(list->atm);
  // list->cnt_recycle = 0;
  free(list);
  return(ZOK);
}

ZINLINE void zlist_getnode(zlist_t *list, znod_t **nod){
  *nod = list->recycle;
  if(*nod){
    list->recycle = (*nod)->next;
    --list->cnt_recycle;
  }else{
    *nod = (znod_t*)malloc(sizeof(znod_t));
  }
}

int zlist_push(zcontainer_t cont, zvalue_t in){//push back
  znod_t *nod;
  zlist_t *list;
  int ret;
  ZASSERT(!cont);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm);
  if(ZOK != ret){
    return ret;
  }
  zlist_getnode(list, &nod);
  if(!nod){
    ziatm_unlock(list->atm);
    return(ZMEM_INSUFFICIENT);
  }
  nod->value = in;
  
  if(list->head){
    nod->prev = list->tail;
    nod->next = NULL;
    list->tail->next = nod;
    list->tail = nod;
  }else{
    list->head = list->tail = nod;
    nod->prev = nod->next = NULL;
  }
  ++list->num;
  ziatm_unlock(list->atm);
  return(ZOK);
}
ZINLINE void zlist_recycle(zlist_t *list, znod_t *nod){
  if(list->cnt_recycle < list->max_recycle){
    nod->next = list->recycle;
    list->recycle = nod;
    ++list->cnt_recycle;
  }else{
    free(nod);
  }
}

int zlist_pop(zcontainer_t cont, zvalue_t *out){
  znod_t *nod;
  zlist_t *list;
  int ret;
  ZASSERT(!cont);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm);
  if(ZOK != ret){
    return ret;
  }
  if(list->head){
    nod = list->head;
    *out = nod->value;
    list->head = nod->next;
    --list->num;
    zlist_recycle(list, nod);
    // adapt tail
    if(list->head){
      list->head->prev = NULL;
    }else{
      list->tail = list->head = NULL;
    }
  }else{
    *out = NULL;
    ret = ZNOT_EXIST;
  }
  ziatm_unlock(list->atm);
  return ret;
}

int zlist_pushfront(zcontainer_t cont, zvalue_t in){
  znod_t *nod;
  zlist_t *list;
  int ret;

  ZASSERT(!cont);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm); ZASSERTR(ret);

  zlist_getnode(list, &nod);
  if(!nod){
    ziatm_unlock(list->atm);
    return(ZMEM_INSUFFICIENT);
  }
  nod->value = in;
  
  if(list->head){
    nod->next = list->head;
    nod->prev = NULL;
    list->head->prev = nod;
    list->head = nod;
  }else{
    list->head = list->tail = nod;
    nod->prev = nod->next = NULL;
  }
  ++list->num;
  ziatm_unlock(list->atm);
  return(ZOK);
}

int zlist_popback(zcontainer_t cont, zvalue_t *out){
  znod_t *nod;
  zlist_t *list;
  int ret;
  ZASSERT(!cont);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm);
  if(ZOK != ret){
    return ret;
  }
  if(list->tail){
    nod = list->tail;
    *out = nod->value;
    list->tail = nod->prev;
    --list->num;
    //recycle node
    zlist_recycle(list, nod);
    // adapt tail
    if(list->tail){
      list->tail->next = NULL;
    }else{
      list->tail = list->head = NULL;
    }
  }else{
    *out = NULL;
    ret = ZNOT_EXIST;
  }
  ziatm_unlock(list->atm);
  return ret;
}

int zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare){
  return zlist_push(cont, in);
}

int zlist_erase(zcontainer_t cont, zvalue_t in, zoperate compare){
  zlist_t *list;
  znod_t *nod;
  znod_t *nod1;
  int ret;
  
  ZASSERT(!cont);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm); ZASSERTR(ret);
  
  nod = list->head;
  while(nod){
    if(compare){
      ret = compare(nod->value, NULL, in);
    }else if(nod->value == in){
      ret = ZCMP_EQUAL;
    }else{
      ret = ZEFAIL;
    }
    if(ZECMP_EQUAL == ret){
      // NULL<->head<->nod1<->nod2...nodn<->tail->NULL
      nod1 = nod;
      if(list->head == nod){
	list->head = nod->next;
	if(!list->head){
	  list->tail = NULL;
	}
      }
      if(list->tail == nod){
	list->tail = nod->prev;
	list->tail->next = NULL;
      }
      nod = nod->next;
      zlist_recycle(list, nod1);
    }
  }
  ziatm_unlock(list->atm);
  return(ZOK);
}

int zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
  znod_t *nod;
  zlist_t *list;
  int ret;
  ZASSERT(!cont || !op);
  list = (zlist_t*)cont;
  ret = ziatm_lock(list->atm);
  ZASSERTR(ret);
  nod = list->head;
  while(nod){
    ret = op(nod->value, NULL, hint);
    if(ret == ZCMD_STOP){
      break;
    }
    nod = nod->next;
  }
  ziatm_unlock(list->atm);
  return(ZOK);
}
