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
#include "export.h"
#include <zit/base/trace.h>
#include <zit/memory/alloc.h>
#include <zit/memory/mem_pool.h>
#include <zit/memory/obj_pool.h>

static zmem_pool_t *s_mem_pool;

zerr_t zalloc_create(zalloc_t **alloc, int block_size,
                             int capacity, int limit_mem){
    zerr_t ret = ZEOK;
    zqueue_t *mque = NULL;
    zqueue_t *rque = NULL;
    zalloc_t *alc = (zalloc_t*)calloc(1, sizeof(zalloc_t));
    zqueue_create(&mque, capacity, block_size);
    zqueue_create(&rque, 512, sizeof(zvalue_t));
    if(alc && mque && rque){
        *alloc = alc;
        alc->chk_cnt = 0;
        alc->max_mem = (limit_mem > 1024*1024 && limit_mem <= 512*1024*1024)
            ? limit_mem : 32 * 1024 * 1024;
        alc->mem_que = mque;
        alc->rec_que = rque;
        zspin_init(&alc->spin_mo);
        zspin_init(&alc->spin_ri);
        zspin_init(&alc->spin_ro);
        mque->end_pos = mque->chunk_length; /* set full memory */
    }else{
        ret = ZEMEM_INSUFFICIENT;
        free(alc);
        zqueue_destroy(mque);
        zqueue_destroy(rque);
    }
    return ret;
}

zerr_t zalloc_destroy(zalloc_t *alloc){
    if(alloc){
        zspin_fini(&alloc->spin_mo);
        zspin_fini(&alloc->spin_ri);
        zspin_fini(&alloc->spin_ro);
        zqueue_destroy(alloc->mem_que);
        zqueue_destroy(alloc->rec_que);
        free(alloc);
    }
    return ZEOK;
}

void zmem_pool_init(){
    zmem_pool_create(&s_mem_pool);
}

void zmem_pool_fini(){
    zmem_pool_destroy(s_mem_pool);
}

zmem_pool_t *zmem_pool(){
    return s_mem_pool;
}

zerr_t zmem_pool_free(ZOP_ARG){
    free(((zmem_pool_item_t*)in)->buf);
    return ZOK;
}

zerr_t zobj_pool_create(zobj_pool_t **pool, uint32_t fixed_size,
                        int capacity, int limit_mem){
    zerr_t ret = ZMEM_INSUFFICIENT;
    zobj_pool_t *pl = NULL;
    zalloc_t *alc = NULL;
    do{
        pl = (zobj_pool_t*)calloc(1, sizeof(zobj_pool_t));
        if(!pl){
            break;
        }

        if(ZOK != (ret = zalloc_create(&alc, fixed_size,
                                       capacity, limit_mem))){
            break;
        }
        *pool = pl;
        pl->fixed_size = fixed_size;
        pl->pool_fixed = (zptr_t)alc;
        pl->pool_unfixed = (zptr_t)s_mem_pool;
        ret = ZOK;
    }while(0);
    if(ZOK != ret && pl){
        free(pl);
    }
    ZERRCX(ret);
    return ret;
}
zerr_t zobj_pool_destroy(zobj_pool_t *pool){
    if(pool){
        zalloc_destroy(pool->pool_fixed);
        free(pool);
    }
    ZERRC(ZOK);
    return ZOK;
}

static zerr_t zobj_mem_clone(ZOP_ARG){
    zerr_t ret = ZOK;
    zobj_t *obj = (zobj_t*)in;
    zalloc_t *alc = (zalloc_t*)((zobj_pool_t*)obj->alloc)->pool_fixed;

    if(ZOK == (ret = zalloc_pop(alc, out))){
        memcpy(*out, obj, alc->mem_que->value_size);
    }else{
        ZERRC(ret);
    }
    return ret;
}

static zerr_t zobj_mem_release(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    zalloc_t *alc = (zalloc_t*)((zobj_pool_t*)obj->alloc)->pool_fixed;

    return zalloc_push(alc, &in);
}

static zerr_t zobj_mem_clone_unfixed(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    zmem_pool_t *alc = (zmem_pool_t*)((zobj_pool_t*)obj->alloc)->pool_unfixed;
    zerr_t ret = ZOK;

    if(ZOK == (ret = zmem_pool_pop(alc, out, obj->size))){
        memcpy(*out, obj, obj->size);
    }else{
        ZERRC(ret);
    }
    return ret;
}

static zerr_t zobj_mem_release_unfixed(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    zmem_pool_t *alc = (zmem_pool_t*)((zobj_pool_t*)obj->alloc)->pool_unfixed;

    return zmem_pool_push(alc, &in, obj->size);
}

static zerr_t zobj_ref_clone(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    *out = in;
    return 2 > zatm_inc(&obj->ref) ? ZNOT_EXIST : ZOK;
}

static zerr_t zobj_ref_release(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    zalloc_t *alc = (zalloc_t*)((zobj_pool_t*)obj->alloc)->pool_fixed;

    return zatm_dec(&obj->ref) ? ZOK : zalloc_push(alc, &in);
}

static zerr_t zobj_ref_release_unfixed(ZOP_ARG){
    zobj_t *obj = (zobj_t*)in;
    zmem_pool_t *alc = (zmem_pool_t*)((zobj_pool_t*)obj->alloc)->pool_unfixed;

    return zatm_dec(&obj->ref) ? ZOK : zmem_pool_push(alc, &in, obj->size);
}

zerr_t zobj_pool_pop(zobj_pool_t *pool, zobj_t **obj,
                           int32_t clone_mode){
    zalloc_t *alc = pool->pool_fixed;
    zerr_t ret = ZOK;
    zobj_t *object = NULL;

    if(ZOK == (ret = zalloc_pop(alc, (zptr_t*)&object))){
        *obj = object;
        if(clone_mode == ZCLONE_MODE_MEMCPY){
            object->clone = zobj_mem_clone;
            object->release = zobj_mem_release;
        }else{
            zatm_xchg(&object->ref, 1);
            object->clone = zobj_ref_clone;
            object->release = zobj_ref_release;
        }
        object->size = pool->fixed_size;
        object->alloc = (zptr_t)pool;
    }
    ZERRCX(ret);
    return ret;
}

zerr_t zobj_pool_push(zobj_pool_t *pool, zobj_t **obj){
    return zalloc_push((zalloc_t*)pool->pool_fixed, (zptr_t*)obj);
}


zerr_t zobj_pool_pop_unfixed(zobj_pool_t *pool, zobj_t **obj
                             ,int32_t clone_mode, uint32_t size){
    zerr_t ret = ZOK;
    zobj_t *object;

    if(ZOK == (ret = zmem_pool_pop((zmem_pool_t*)pool->pool_unfixed,
                                   (zptr_t*)&object, size))){
        *obj = object;
        if(clone_mode == ZCLONE_MODE_MEMCPY){
            object->clone = zobj_mem_clone_unfixed;
            object->release = zobj_mem_release_unfixed;
        }else{
            zatm_xchg(&object->ref, 1);
            object->clone = zobj_ref_clone;
            object->release = zobj_ref_release_unfixed;
        }
        object->size = size;
        object->alloc = (zptr_t)pool;
    }
    ZERRCX(ret);
    return ret;
}

zerr_t zobj_pool_push_unfixed(zobj_pool_t *pool, zobj_t **obj){
    return zmem_pool_push((zmem_pool_t*)pool->pool_unfixed, (zptr_t*)obj, (*obj)->size);
}
