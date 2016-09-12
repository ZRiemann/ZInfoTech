/**@file zit/src/framework.c
 * @brief zit framework implements
 * @note
 * @email 25452483@qq.com
 * @history 2016-9-12 ZRiemann found
 */
#include "export.h"
#include <zit/framework/object.h>
#include <zit/base/error.h>
#include <stdlib.h>

int zobj_free(OPARG){
  free(in);
  return(ZOK);
}

int zobj_init(zobj_t *obj, zobj_t *parent, zobj_type_t type,\
	  zoperate op, zoperate cl, zoperate re, zoperate se){
  ZASSERT(!obj);
  obj->parent = parent;
  obj->type = type;
  obj->operate = op;
  obj->clone = cl;
  obj->release = re;
  obj->serialize = se;
  return(ZOK);
}


ZAPI int zdev_init(zdev_t *dev,zobj_type_t type,  zoperate init, zoperate fini, zoperate run, zoperate stop){
  ZASSERT(!dev);
  zobj_initx((zobj_t*)dev, type, NULL);
  dev->open = NULL;
  dev->close = NULL;
  dev->init = init;
  dev->fini = fini;
  dev->run = run;
  dev->stop = stop;
  return(ZOK);
}
