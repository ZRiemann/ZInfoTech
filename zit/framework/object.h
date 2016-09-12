#ifndef _ZFR_OBJEDT_H_
#define _ZFR_OBJEDT_H_
/**@file zit/framework/object.h
 * @biref zfr object definitions
 * @note
 * @email 25452483@qq.com
 * @history 2016-9-12 ZRiemann found
 */
#include <zit/base/type.h>

ZC_BEGIN

#define ZOBJ_TYPE_DUMMY 0
#define ZOBJ_TYPE_STOP 1
#define ZOBJ_TYPE_RESTART 2
#define ZOBJ_TYPE_USER 1024 // 1024+* for user define object types

typedef uint32_t zobj_type_t;

typedef struct zobject_s{
  zobj_type_t type;
  struct zobject_s *parent;  // support inherit, parent object
  zoperate operate;
  zoperate clone;
  zoperate release;
  zoperate serialize;
}zobj_t;

ZAPI int zobj_free(OPARG);
ZAPI int zobj_init(zobj_t *obj, zobj_t *parent, zobj_type_t type,\
	       zoperate op, zoperate cl, zoperate re, zoperate se);
#define zobj_initx(obj, type, op) zobj_init(obj, NULL, type, op, NULL, zobj_free, NULL)

// inherit from zobj_t
typedef struct zdevict_s{
  zobj_t obj;
  // device operates
  zoperate open;
  zoperate close;
  // server operates
  zoperate init;
  zoperate fini;
  zoperate run;
  zoperate stop;
}zdev_t;
#define dev_type obj.type
#define dev_parent obj.parent
#define dev_release obj.release
#define dev_operate obj.operate
#define dev_clone obj.clone
#define dev_serialize obj.serialize

ZAPI int zdev_init(zdev_t *dev, zobj_type_t type,  zoperate init, zoperate fini, zoperate run, zoperate stop);

ZC_END

#endif
