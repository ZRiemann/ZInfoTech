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

ZAPI zerr_t zobj_free(ZOP_ARG);
ZAPI zerr_t zobj_init(zobj_t *obj, zobj_t *parent, zobj_type_t type,\
	       zoperate op, zoperate cl, zoperate re, zoperate se);
#define zobj_initx(obj, type, op) zobj_init(obj, NULL, type, op, NULL, NULL, NULL)

// inherit from zobj_t
/*
  level0
  level1
  level2
*/
typedef struct zdevice_s{
  zobj_t obj;
  zcontainer_t *owner; // parents
  zcontainer_t *mount; // childrens
  zstatus_t status;
  // server/device operates
  zoperate init;  // dev open()
  zoperate fini;  // dev close()
  zoperate run;
  zoperate stop;
}zdev_t;
typedef zdev_t zsvr_t; // server is device

#define dev_type obj.type
#define dev_parent obj.parent
#define dev_release obj.release
#define dev_operate obj.operate
#define dev_clone obj.clone
#define dev_serialize obj.serialize

ZAPI zerr_t zdev_init(zdev_t *dev, zobj_type_t type,  zoperate init, zoperate fini, zoperate run, zoperate stop);
ZAPI zerr_t zdev_attach(zdev_t *self, zdev_t *mount);
ZAPI zerr_t zdev_detach(zdev_t *self, zdev_t *mount);
// operate all mounts
ZAPI zerr_t zdev_mount_init(zdev_t *dev, ZOP_ARG);
ZAPI zerr_t zdev_mount_fini(zdev_t *dev, ZOP_ARG);
ZAPI zerr_t zdev_mount_run(zdev_t *dev, ZOP_ARG);
ZAPI zerr_t zdev_mount_stop(zdev_t *dev, ZOP_ARG);
ZC_END

#endif
