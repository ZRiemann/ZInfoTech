#ifndef _ZFR_OBJEDT_H_
#define _ZFR_OBJEDT_H_
/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
/**
 * @file zit/framework/object.h
 * @biref zit framework object definitions
 * @author Z.Riemann <Z.Riemann.g@gmail.com>
 */
#include <zit/base/type.h>
#include <zit/base/atomic.h>

ZC_BEGIN

#define ZOBJ_TYPE_DUMMY 0
#define ZOBJ_TYPE_STOP 1
#define ZOBJ_TYPE_RESTART 2
#define ZOBJ_TYPE_USER 1024 /** 1024+* for user define object types */

#define ZCLONE_MODE_REF 0 /** clone by reference count */
#define ZCLONE_MODE_MEMCPY 1 /** clone by new memory and copy */

typedef uint32_t zotype_t; /** object type */
typedef uint32_t zoid_t; /** object identify */
typedef zatm32_t zstatus_t; /** device status */

/**
 * @brief zit object definition
 */
typedef struct zobject_s{
    /* resource data */
    zotype_t type; /** object type */
    zoid_t id; /** object identify */
    zatm32_t status; /** resource status */

    /* allocator data */
    zatm32_t ref; /** reference count */
    uint32_t size; /** object size */
    zptr_t alloc; /** allocator pointer */

    /* resource functions */
    zoperate operate; /** operate function */
    zoperate clone; /** clone object */
    zoperate release; /** release object */
    zoperate serialize; /** serialize object */

    /* char data[];  user definded */
}zobj_t;

zinline char* zobj_extern(char *obj){
	return obj + sizeof(zobj_t);
}

/**
 * @brief device inherit object
 */
typedef struct zdevice_s{
    zobj_t obj;
    // server/device operates
    zoperate init;  /** open/initialize device */
    zoperate fini;  /** close/finish device  */
    zoperate run; /** run device */
    zoperate stop; /** stop device */
}zdev_t;

typedef zdev_t zsvr_t; // server is device

#define dev_type obj.type
#define dev_id obj.id
#define dev_release obj.release
#define dev_operate obj.operate
#define dev_clone obj.clone
#define dev_serialize obj.serialize

ZC_END

#endif
