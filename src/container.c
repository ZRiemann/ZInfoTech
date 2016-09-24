#include "export.h"
#include <zit/base/container.h>
#include <zit/base/list.h>
#include <zit/base/queue.h>
#include <zit/base/error.h>
#include <stdlib.h>
#include <string.h>

typedef int (*zcont_destroy)(zcontainer_t cont, zoperate release);
typedef int (*zcont_push)(zcontainer_t cont, zvalue_t in);
typedef int (*zcont_pop)(zcontainer_t cont, zvalue_t *out);
typedef int (*zcont_pushfront)(zcontainer_t cont, zvalue_t in);
typedef int (*zcont_popback)(zcontainer_t cont, zvalue_t *out);
typedef int (*zcont_insert)(zcontainer_t cont, zvalue_t in, zoperate compare);
typedef int (*zcont_erase)(zcontainer_t cont, zvalue_t in, zoperate compare);
typedef int (*zcont_foreach)(zcontainer_t cont, zoperate op, zvalue_t hint);
typedef zsize_t (*zcont_size)(zcontainer_t cont);

typedef struct zcontainer_s{
  zcontainer_t cont;
  zcont_destroy destroy;
  zcont_push push;
  zcont_pop pop;
  zcont_pushfront pushfront;
  zcont_popback popback;
  zcont_insert insert;
  zcont_erase erase;
  zcont_foreach foreach;
  zcont_size size;
}zcont_t;

int zcontainer_create(zcontainer_t *cont, int type){
  int ret = ZEOK;
  zcont_t *cnt;
  ZASSERT(!cont);
  *cont = malloc(sizeof(zcont_t));
  //if(!*cont)return(ZMEM_INSUFFICIENT);
  ZASSERTC(!*cont, ZEMEM_INSUFFICIENT);
  cnt = (zcont_t*)(*cont);
  if(type == ZCONTAINER_LIST){
    ret = zlist_create(&cnt->cont);
    if(ZOK == ret){
      cnt->destroy = zlist_destroy;
      cnt->push = zlist_push;
      cnt->pop = zlist_pop;
      cnt->pushfront = zlist_pushfront;
      cnt->popback = zlist_popback;
      cnt->insert = zlist_insert;
      cnt->erase = zlist_erase;
      cnt->foreach = zlist_foreach;
      cnt->size = zlist_size;
    }else{
      free(cnt);
      *cont = NULL;
    }
  }else{
    ret = zque_create(&cnt->cont);
    if(ZOK == ret){
      cnt->destroy = zque_destroy;
      cnt->push = zque_push;
      cnt->pop = zque_pop;
      cnt->pushfront = zque_pushfront;
      cnt->popback = zque_popback;
      cnt->insert = zque_insert;
      cnt->erase = zque_erase;
      cnt->foreach = zque_foreach;
      cnt->size = zque_size;
    }else{
      free(cnt);
      *cont = NULL;
    }
  }
  return(ZOK);
}
int zcontainer_destroy(zcontainer_t cont, zoperate release){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->destroy(cnt->cont, release));
}
int zcontainer_push(zcontainer_t cont, zvalue_t in){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->push(cnt->cont, in));
}
int zcontainer_pop(zcontainer_t cont, zvalue_t *out){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->pop(cnt->cont, out));
}
int zcontainer_pushfront(zcontainer_t cont, zvalue_t in){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->pushfront(cnt->cont, in));
}
int zcontainer_popback(zcontainer_t cont, zvalue_t *out){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->popback(cnt->cont, out));
}
int zcontainer_insert(zcontainer_t cont, zvalue_t in, zoperate compare){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->insert(cnt->cont, in, compare));
}
int zcontainer_erase(zcontainer_t cont, zvalue_t in, zoperate compare){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->erase(cnt->cont, in, compare));
}
int zcontainer_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->foreach(cnt->cont, op, hint));
}

zsize_t zcontainer_size(zcontainer_t cont){
  zcont_t *cnt = (zcont_t*)cont;
  ZASSERT(!cnt);
  return(cnt->size(cnt->cont));
}
