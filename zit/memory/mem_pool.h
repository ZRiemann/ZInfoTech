#ifndef _Z_MEM_POOL_H_
#define _Z_MEM_POOL_H_
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
 * @file zit/memory/mem_pool.h
 * @brief Memory pool
 *
 * @par
 *      Unfixed size memory pool,
 *      Garbage collection mechanism by user implement.
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/container/rbtree.h>

ZC_BEGIN

typedef zbtree_t zmem_pool_t;

typedef struct zmemory_pool_item_s{
    uint32_t buf_size; /** memory buffer size, power of 2 */
    zptr_t buf; /** memory buffer address */
}zmem_pool_item_t;

/**
 * @brief init global memory pool
 */
ZAPI void zmem_pool_init();
/**
 * @brief finish global memory pool
 */
ZAPI void zmem_pool_fini();
/**
 * @brief get global memory pool pointer
 * @return memory pool pointer
 */
ZAPI zmem_pool_t *zmem_pool();

zinline zerr_t zmem_pool_create(zmem_pool_t **pool){
    return zrbtree_create(pool, sizeof(zmem_pool_item_t), 32, NULL, ZTRUE);
}

ZAPI zerr_t zmem_pool_free(ZOP_ARG);
zinline zerr_t zmem_pool_destroy(zmem_pool_t *pool){
    zrbtree_foreach(pool->root, 0, zmem_pool_free, NULL);
    return zrbtree_destroy(pool);
}

zinline uint32_t power_of2(uint32_t size){
    uint32_t old = size;
    size <<= 1;
    while(size){
        old = size;
        size &= size-1;
    }
    return old;
}

static zerr_t zmem_pool_cmp(ZOP_ARG){
    /* copy key data from node data */
    memcpy(hint, in, sizeof(zmem_pool_item_t));
    /* pop list head, just return equal */
    return ZEQUAL;
}

zinline zerr_t zmem_pool_pop(zmem_pool_t *pool, zptr_t *ptr, uint32_t size){
    zerr_t ret = ZOK;
    char buf[128] = {0};
    zbtnode_t *key = (zbtnode_t*)buf;

    size = power_of2(size);
    ((zmem_pool_item_t*)key->data)->buf_size = size;
    if(ZOK == (ret = zrbtree_erasex(pool, key, zmem_pool_cmp))){
        *ptr = ((zmem_pool_item_t*)key->data)->buf;
    }else{
        *ptr = calloc(1, size);
        ret = *ptr ? ZOK : ZMEM_INSUFFICIENT;
    }
    return ret;
}

zinline zerr_t zmem_pool_push(zmem_pool_t *pool, zptr_t *ptr, uint32_t size){
    zbtnode_t *nod = NULL;

    if(ZOK == zrbtree_get_node(pool, &nod)){
        zmem_pool_item_t *ufn = (zmem_pool_item_t*)nod->data;
        ufn->buf_size = power_of2(size);
        ufn->buf = *ptr;
        zrbtree_insert(pool, nod);
    }else{
        /* Can not alloc node, just free */
        free(*ptr);
    }
    return ZOK;
}

ZC_END
#endif
