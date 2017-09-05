#ifndef _Z_CONT_ALLOC_H_
#define _Z_CONT_ALLOC_H_
/**
 * @file zit/container/base/alloc.h
 * @brief A container memory allocator
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/container/base/queue.h>
#include <zit/thread/spin.h>
#include <stdlib.h>

ZC_BEGIN
typedef struct zbase_alloc_s{
    zqueue_t *mem_que; /** memory queue, user define structure*/
    zqueue_t *rec_que; /** recycle queue, pointer to user define structure*/
    zspinlock_t spin_mo; /** spin lock to memory queue out */
    zspinlock_t spin_ri; /** spin lock to recycle queue in */
    zspinlock_t spin_ro; /** spin lock to recycle queue out */
}zalloc_t;

zinline zerr_t zalloc_create(zalloc_t **alloc, int block_size, int capacity){
    zerr_t ret = ZEOK;
    zqueue_t *mque = NULL;
    zqueue_t *rque = NULL;
    zalloc_t *alc = (zalloc_t*)calloc(1, sizeof(zalloc_t));
    zqueue_create(&mque, capacity, block_size);
    zqueue_create(&rque, 512, sizeof(zvalue_t));
    if(alc && mque && rque){
        *alloc = alc;
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

zinline zerr_t zalloc_destroy(zalloc_t *alloc){
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

zinline zerr_t zalloc_pop(zalloc_t *alloc, zvalue_t *val){
    zqueue_t *mque = alloc->mem_que;
    /* 1. pop recycle queue */
    if(ZEOK == zqueue_lock_pop(alloc->rec_que, val, &alloc->spin_ro)){
        *val = *((zptr_t*)*val);
    }else{
        /* 2. allocte from  memory queue */
        zspin_lock(&alloc->spin_mo);
        if(mque->end_pos == 0){
            /* 2.1.a no more free memory and allocte  a chunk */
            zchkx_t *chunk = ZCHUNK_ALLOCX(mque->chunk_length);
            if(!chunk){
                zspin_unlock(&alloc->spin_mo);
                return ZEMEM_INSUFFICIENT;
            }
            chunk->prev = mque->end_chunk;
            chunk->next = NULL;
            mque->end_chunk->next = chunk;
            mque->end_chunk = chunk;
            mque->end_pos = mque->chunk_length;
        }
        /* 2.1.b has free memory */
        mque->end_pos -= mque->value_size;
        *val = (zvalue_t)(mque->end_chunk->value + mque->end_pos);
        zspin_unlock(&alloc->spin_mo);
    }
    memset(*val, 0, mque->value_size);
    return ZEOK;
}

zinline zerr_t zalloc_push(zalloc_t *alloc, zvalue_t *val){
    return zqueue_lock_push(alloc->rec_que, val, &alloc->spin_ri);
}

ZC_END
#endif