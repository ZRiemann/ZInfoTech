#ifndef _ZBASE_QUEUE_H_
#define _ZBASE_QUEUE_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
/*
  CAUTION: Linux64 use int64_t for value_t, if int will cause memary overwrie;
  int: 0~31
  queue ptr: 32~95
  queue->pop(queue,int) ;error:overwrite 32~63 bit.
  thread safe
*/
ZC_BEGIN

ZAPI int zque_create(zcontainer_t *cont);
ZAPI int zque_destroy(zcontainer_t cont, zoperate release);
ZAPI int zque_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI int zque_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI int zque_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI int zque_popback(zcontainer_t cont, zvalue_t *out);
ZAPI int zque_insert(zcontainer_t cont, zvalue_t in, zoperate compare);
ZAPI int zque_erase(zcontainer_t cont, zvalue_t in, zoperate compare);
ZAPI int zque_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI zsize_t zque_size(zcontainer_t cont);

ZAPI int zque_back(zcontainer_t cont, zvalue_t *out);
ZAPI int zque_front(zcontainer_t cont, zvalue_t *out);

ZAPI int zque_swap(zcontainer_t *cont1, zcontainer_t *cont2);

ZC_END

#endif
