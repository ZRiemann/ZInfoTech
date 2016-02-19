#ifndef _ZBASE_QUEUE_H_
#define _ZBASE_QUEUE_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <zit/base/atomic.h>
/*
  CAUTION: Linux64 use int64_t for value_t, if int will cause memary overwrie;
  int: 0~31
  queue ptr: 32~95
  queue->pop(queue,int) ;error:overwrite 32~63 bit.
*/
#ifdef __cplusplus
extern "C" {
#endif

#define ZQUEUE_SIZE 1024
typedef struct zchunk_t{
  zvalue_t v[ZQUEUE_SIZE];
  struct zchunk_t* next;
  struct zchunk_t* prev;
}zchnk_t;

typedef struct zqueue_t{
  zchnk_t* chnkback;
  zchnk_t* chnkfront;
  zatmc_t* atm; ///< spare chunk
  int posfront;
  int posback;
}zque_t;

ZEXP int zqueue_create(zque_t** que);
ZEXP int zqueue_destroy(zque_t** que);
// extern queue api
ZEXP int zqueue_pushback(zque_t* que, zvalue_t value);
ZEXP int zqueue_pushfront(zque_t* que, zvalue_t value);
ZEXP int zqueue_popback(zque_t* que, zvalue_t* value);
ZEXP int zqueue_popfront(zque_t* que, zvalue_t* value);
ZEXP int zqueue_foreach(zque_t* que, ztsk_t* tsk);

#ifdef __cplusplus
}
#endif

#endif
