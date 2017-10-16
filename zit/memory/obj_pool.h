#ifndef _Z_MEM_OBJ_POOL_H_
#define _Z_MEM_OBJ_POOL_H_
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
 * @file zit/memory/obj_pool.h
 * @brief Object pool
 */
#include <zit/framework/object.h>

/**
 * @brief A object memory pool
 */
typedef struct zobject_allocator_t{
    zptr_t pool_fixed; /** fixed size memory pool */
    uint32_t fixed_size; /** fixed memory block size */
    zptr_t pool_unfixed; /** unfixed size pool */

    zoperate allocate; /** allocate function */
}zobj_pool_t;

ZAPI zerr_t zobj_pool_create(zobj_pool_t **pool, uint32_t fixed_size,
                             int capacity, int limit_mem);
ZAPI zerr_t zobj_pool_destroy(zobj_pool_t *pool);

/**
 * @breif pop a memory block from fixed pool
 * @param [in] pool pool pointer
 * @param [out] obj point to object pointer
 * @param [in] object clone mode, ZCLONE_MODE_REF/MEMCPY
 */
ZAPI zerr_t zobj_pool_pop(zobj_pool_t *pool, zobj_t **obj
                                  ,int32_t clone_mode);
/**
 * @brief push a memory block back to unfixed pool, pop by zobj_pool_pop()
 * @param [in] pool pool pointer
 * @param [in] obj object pointer address
 */
ZAPI zerr_t zobj_pool_push(zobj_pool_t *pool, zobj_t **obj);

/**
 * @breif pop a memory block from unfixed pool
 * @param [in] pool pool pointer
 * @param [out] obj point to object pointer
 * @param [in] object clone mode, ZCLONE_MODE_REF/MEMCPY
 * @param [in] buffer size;
 */
ZAPI zerr_t zobj_pool_pop_unfixed(zobj_pool_t *pool, zobj_t **obj
                                  ,int32_t clone_mode, uint32_t size);

/**
 * @brief push a memory block back to unfixed pool, pop by zobj_pool_pop()
 * @param [in] pool pool pointer
 * @param [in] obj object pointer address
 */
ZAPI zerr_t zobj_pool_push_unfixed(zobj_pool_t *pool, zobj_t **obj);

#endif
