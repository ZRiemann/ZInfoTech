/**@file zit/src/list.c
 * @brief general list implement
 * @note
 */
#include "export.h"
#include <zit/base/list.h>
#include <zit/base/error.h>
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
    list->tail = list->tail->next;
    if(release){
      release(OPIN(nod->value));
    }
    free(nod);
  }
  // list->cnt_recycle = 0;
  free(list);
  return(ZOK);
}

int zlist_push(zcontainer_t cont, zvalue_t in){//push back
  znod_t *nod;
  zlist_t *list;

  ZASSERT(!cont);
  list = (zlist_t*)cont;
  nod = list->recycle;
  if(nod){
    list->recycle = nod->next;
    --list->cnt_recycle;
  }else{
    nod = (znod_t*)malloc(sizeof(znod_t));
  }
  if(!nod)return(ZMEM_INSUFFICIENT);
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
  return(ZOK);
}

int zlist_pop(zcontainer_t cont, zvalue_t *out){
  znod_t *nod;
  zlist_t *list;

  ZASSERT(!cont);
  list = (zlist_t*)cont;
  if(list->head){
    nod = list->head;
    *out = nod->value;
    list->head = nod->next;
    --list->num;
    //recycle node
    if(list->cnt_recycle < list->max_recycle){
      nod->next = list->recycle;
      list->recycle = nod;
      ++list->cnt_recycle;
    }else{
      free(nod);
    }
    // adapt tail
    if(list->head){
      list->head->prev = NULL;
    }else{
      list->tail = list->head = NULL;
    }
  }else{
    *out = NULL;
    return(ZNOT_EXIST);
  }
  return(ZOK);
}

int zlist_pushfront(zcontainer_t cont, zvalue_t in){
  znod_t *nod;
  zlist_t *list;

  ZASSERT(!cont);
  list = (zlist_t*)cont;
  nod = list->recycle;
  if(nod){
    list->recycle = nod->next;
    --list->cnt_recycle;
  }else{
    nod = (znod_t*)malloc(sizeof(znod_t));
  }
  if(!nod)return(ZMEM_INSUFFICIENT);
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
  return(ZOK);
}

int zlist_popback(zcontainer_t cont, zvalue_t *out){
  znod_t *nod;
  zlist_t *list;
  ZASSERT(!cont);
  list = (zlist_t*)cont;
  if(list->tail){
    nod = list->tail;
    *out = nod->value;
    list->tail = nod->prev;
    --list->num;
    //recycle node
    if(list->cnt_recycle < list->max_recycle){
      nod->next = list->recycle;
      list->recycle = nod;
      ++list->cnt_recycle;
    }else{
      free(nod);
    }
    // adapt tail
    if(list->tail){
      list->tail->next = NULL;
    }else{
      list->tail = list->head = NULL;
    }
  }else{
    *out = NULL;
    return(ZNOT_EXIST);
  }
  return(ZOK);
}

int zlist_insert(zcontainer_t list, zvalue_t in, zoperate compare){
  return(ZNOT_SUPPORT);
}

int zlist_erase(zcontainer_t list, zvalue_t in, zoperate compare){
  return(ZNOT_SUPPORT);
}

int zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
  znod_t *nod;
  zlist_t *list;
  ZASSERT(!cont || !op);
  list = (zlist_t*)cont;
  nod = list->head;
  while(nod){
    op(nod->value, NULL, hint);
    nod = nod->next;
  }
  return(ZOK);
}
