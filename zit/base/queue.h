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
ZC_BEGIN

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

ZAPI int zqueue_create(zque_t** que);
ZAPI int zqueue_destroy(zque_t** que);
ZAPI int zqueue_init(zque_t* que);
ZAPI int zqueue_uninit(zque_t* que);
// extern queue api
ZAPI int zqueue_pushback(zque_t* que, zvalue_t value);
ZAPI int zqueue_pushfront(zque_t* que, zvalue_t value);
ZAPI int zqueue_popback(zque_t* que, zvalue_t* value);
ZAPI int zqueue_popfront(zque_t* que, zvalue_t* value);
ZAPI int zqueue_foreach(zque_t* que, ztsk_t* tsk);

ZAPI int zque_create(zcontainer_t *list);
ZAPI int zque_destroy(zcontainer_t list, zoperate release);
ZAPI int zque_push(zcontainer_t list, zvalue_t in); // push back
ZAPI int zque_pop(zcontainer_t list, zvalue_t *out); // pop front
ZAPI int zque_pushfront(zcontainer_t list, zvalue_t in);
ZAPI int zque_popback(zcontainer_t list, zvalue_t *out);
ZAPI int zque_insert(zcontainer_t list, zvalue_t in, zoperate compare);
ZAPI int zque_erase(zcontainer_t list, zvalue_t in, zoperate compare);
ZAPI int zque_foreach(zcontainer_t list, zoperate op, zvalue_t hint);

ZC_END

#endif
