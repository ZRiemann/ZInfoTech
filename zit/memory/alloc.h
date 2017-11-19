#ifndef _Z_CONT_ALLOC_H_
#define _Z_CONT_ALLOC_H_
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
 * @file zit/container/alloc.h
 * @brief Memory allocator
 *
 * @par design pattern
 *      -# Support fixed and unfixed memory poll
 *      -# Support memory poll size control
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/container/queue.h>
#include <zit/thread/spin.h>
#include <stdlib.h>

ZC_BEGIN

/**
 * @brief A fixed size memory block allocator
 */
typedef struct zbase_allocator_s{
    /* fixed size memory pool */
    zqueue_t *mem_que; /** memory queue, user define structure*/
    zqueue_t *rec_que; /** recycle queue, pointer to user define structure*/
    zspinlock_t spin_mo; /** spin lock to memory queue out */
    zspinlock_t spin_ri; /** spin lock to recycle queue in */
    zspinlock_t spin_ro; /** spin lock to recycle queue out */
    /* control max memory buffered */
    uint32_t chk_cnt; /** memory chunk count */
    uint32_t max_mem; /** max memory size, default 32MB */
}zalloc_t;

/**
 * @brief Create a allocator
 * @param [out] alloc pointer pointer to allocator
 * @param [in] block_size memory block size
 * @param [in] memory chunk capacity
 * @param [in] limit_mem limited max memory allocte
 * @return ZEOK success
 *         ZEMEM_INSUFFICIENT memory insufficient
 */
ZAPI zerr_t zalloc_create(zalloc_t **alloc, int block_size,
                             int capacity, int limit_mem);
/**
 * @brief Destroy allocator, created by zalloc_create
 * @param [in] alloc allocator pointer
 * @return ZOK success
 */
ZAPI zerr_t zalloc_destroy(zalloc_t *alloc);

/**
 * @brief pop a memory block from allocator
 * @param [in] alloc pointe to zalloc_t
 * @param [out] ptr pointe to pointer
 * @return ZEOK pop OK
 *         ZEMEM_OUTOFBOUNDS memory buffer is max
 *         ZEMEM_INSUFFICIENT memory insufficient
 */
zinline zerr_t zalloc_pop(zalloc_t *alloc, zptr_t *ptr){
    /* 1. pop recycle queue */
    if(ZEOK == zqueue_lock_front(alloc->rec_que, ptr, &alloc->spin_ro)){
        *ptr = *((zptr_t*)*ptr);
        zqueue_unlock_front(alloc->rec_que, &alloc->spin_ro);
    }else{
        zqueue_t *mque = alloc->mem_que;
        /* 2. allocte from  memory queue */
        zspin_lock(&alloc->spin_mo);
        if(mque->end_pos == 0){
            /* 2.1.a no more free memory and allocte  a chunk */
            zchk_t *chunk = NULL;

            if(alloc->max_mem <
               (mque->chunk_size * mque->value_size * alloc->chk_cnt)){
                /* Control max memory pool size
                 * If buffered memory > max_mem maybe memory leak,
                 * or IO speed not match need to redesign.
                 */
                zspin_unlock(&alloc->spin_mo);
                return ZEMEM_OUTOFBOUNDS;
            }

            chunk = ZCHUNK_ALLOCX(mque->chunk_length);
            if(!chunk){
                /* not enough memory */
                zspin_unlock(&alloc->spin_mo);
                return ZEMEM_INSUFFICIENT;
			}else{
				memset(chunk, 0, mque->chunk_length);
			}

            ++alloc->chk_cnt;
            chunk->prev = mque->end_chunk;
            chunk->next = NULL;
            mque->end_chunk->next = chunk;
            mque->end_chunk = chunk;
            mque->end_pos = mque->chunk_length;
        }
        /* 2.1.b has free memory */
        mque->end_pos -= mque->value_size;
        *ptr = (zvalue_t)(mque->end_chunk->value + mque->end_pos);
        zspin_unlock(&alloc->spin_mo);
    }
    //memset(*ptr, 0, alloc->mem_que->value_size);
    return ZEOK;
}

/**
 * @brief Push back the block pointer address to allocator
 * @param [in] alloc pointer to the allocator
 * @param [in] ptr pointer address
 * @return ZOK success
 *         ZEMEM_INSUFFICIENT can not allocate chunk memory
 */
zinline zerr_t zalloc_push(zalloc_t *alloc, zptr_t *ptr){
    memset(*ptr, 0, alloc->mem_que->value_size);
    return zqueue_lock_push(alloc->rec_que, ptr, &alloc->spin_ri);
}

ZC_END
#endif