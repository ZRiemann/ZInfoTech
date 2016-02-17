#include "export.h"
#include <zit/base/trace.h>
#include <zit/base/error.h>
#include <zit/base/queue.h>
#include <stdlib.h>

/*
  NULL<--node<-->node<-->node<-->node-->NULL
                 ^               ^
                 head            tail
*/
typedef struct zqueue_t{
  znod_t* head;
  znod_t* tail;
  znod_t* chunk; // 1024
  znod_t* spare_chunk;
}zque_t;

int zqueue_create(zcontainer_t* cont){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_destroy(zcontainer_t* cont){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_push(zcontainer_t cont, int back, zvalue_t value, zcompare cmp){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_pop(zcontainer_t cont, int back, zvalue_t* value, zcompare cmp){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_pushback(zcontainer_t cont, zvalue_t value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_pushfront(zcontainer_t cont, zvalue_t value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_popback(zcontainer_t cont, zvalue_t* value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueuq_popfornt(zcontainer_t cont, zvalue_t* value){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}

int zqueue_foreach(zcontainer_t cont, ztsk_t* tsk){
  int ret = ZENOT_SUPPORT;
  
  ZERRC(ret);
  return ret;
}
