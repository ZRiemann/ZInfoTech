#ifndef _ZCONT_QUEUE_H_
#define _ZCONT_QUEUE_H_
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
 * @file zit/container/queue.h
 * @brief a chunk based queue
 *
 * @par
 *   performance
 *   (vm)unbuntu17
 *   $ make/release/zit_test queue 8192 4
 *   push 10,000,000 int32 value, interval:0.156956;
 *   15 nanosecond/push; std::queue 5 nanosecond/push; array 3 nanosecond/push
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/base/atomic.h>
#include <zit/thread/spin.h>
#include <string.h>

ZC_BEGIN
typedef struct zchunk_s{
    struct zchunk_s *next;
    struct zchunk_s *prev;
    char value[];
}zchk_t;

typedef struct zqueue_s{
    zchk_t *begin_chunk;
    zchk_t *end_chunk;
    zchk_t *spare_chunk;
    zatm_t size;
    int32_t begin_pos;
    int32_t end_pos;
    int32_t chunk_size;
    int32_t value_size;
    int32_t chunk_length; /** chunk_size * value_size, space 2 time*/
}zqueue_t;

#if 0
#define ZCHUNK_ALLOCX(size) (zchk_t*)zmem_align64(sizeof(zchk_t) + (size));
#else
zinline zchk_t *ZCHUNK_ALLOCX(size_t size) {
	zchk_t *chk = (zchk_t*)zmem_align64(sizeof(zchk_t) + size);
	if(chk){
		memset(chk, 0, sizeof(zchk_t) + size);
	}
	return chk;
}
#endif
zinline zerr_t zqueue_create(zqueue_t **queue, int32_t chunk_size, int32_t value_size){
    zchk_t *chunk = ZCHUNK_ALLOCX(chunk_size * value_size);
    zqueue_t *que = (zqueue_t*)zmem_align64(sizeof(zqueue_t));
    zatm_t size = zatm_alloc_atm();
    if(chunk && que && size){
        *queue = que;
        memset(que, 0, sizeof(zqueue_t));
        chunk->next = chunk->prev = NULL;
        que->begin_chunk = que->end_chunk = chunk;
        que->size = size;
        que->chunk_size = chunk_size;
        que->value_size = value_size;
        que->chunk_length = chunk_size * value_size;
        *size = 0;
    }else{
        zmem_align_free(chunk);
        zmem_align_free(que);
        zatm_free(size);
        *queue = NULL;
        return ZEMEM_INSUFFICIENT;
    }
    return ZEOK;
}

zinline void zqueue_destroy(zqueue_t *queue){
    zchk_t *chunk = NULL;
    if(!queue){
        return;
    }
    chunk = queue->begin_chunk;
    while(chunk != queue->end_chunk){
        queue->begin_chunk = queue->begin_chunk->next;
        zmem_align_free(chunk);
        chunk = queue->begin_chunk;
    }
    zmem_align_free(chunk);
    chunk = (zchk_t*) zatm_xchg_ptr(&queue->spare_chunk, NULL);
    zmem_align_free(chunk);
    zatm_free(queue->size);
    zmem_align_free(queue);
}

zinline zerr_t zqueue_push(zqueue_t *que, zvalue_t value){
    /*if(que->end_pos < que->chunk_size){*/
    memcpy(que->end_chunk->value + que->end_pos, value, que->value_size);
    que->end_pos += que->value_size;
    if(que->end_pos < que->chunk_length){
        zatm_inc(que->size);
        return ZEOK;
    }else{
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            return ZEMEM_INSUFFICIENT;
        }
        chunk->prev = que->end_chunk;
        chunk->next = NULL;
        que->end_chunk->next = chunk;
        que->end_chunk = chunk;
        que->end_pos = 0;
        zatm_inc(que->size);
    }
    return ZEOK;
}

zinline zerr_t zqueue_pop(zqueue_t *que, zvalue_t *pval){
    if(!*que->size){
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    if(pval){
        *pval = (zvalue_t)(que->begin_chunk->value + que->begin_pos);
    }
    que->begin_pos += que->value_size;
    if(que->begin_pos == que->chunk_length){
        zchk_t *chunk = que->begin_chunk;
        que->begin_chunk = que->begin_chunk->next;
        que->begin_chunk->prev = NULL;
        que->begin_pos = 0;
        chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, chunk);
        zmem_align_free(chunk);
    }
    return ZEOK;
}

zinline zerr_t zqueue_push_front(zqueue_t *que, zvalue_t value){
    que->begin_pos -= que->value_size;
    if(que->begin_pos < 0){
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            return ZEMEM_INSUFFICIENT;
        }
        chunk->next = que->begin_chunk;
        chunk->prev = NULL;
        que->begin_chunk->prev = chunk;
        que->begin_chunk = chunk;
        que->begin_pos = que->chunk_length - que->value_size;
    }
    memcpy(que->begin_chunk->value + que->begin_pos, value, que->value_size);
    zatm_inc(que->size);
}

zinline zerr_t zqueue_pop_back(zqueue_t *que, zvalue_t *pval){
    if(!*que->size){
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    que->end_pos -= que->value_size;
    if(que->end_pos < 0){
        zchk_t *chunk = que->end_chunk;
        que->end_chunk = que->end_chunk->prev;
        que->end_chunk->next = NULL;
        que->end_pos = que->chunk_length - que->value_size;
        chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, chunk);
        zmem_align_free(chunk);
    }
    if(pval){
        *pval = (zvalue_t)(que->end_chunk->value + que->end_pos);
    }
    return ZEOK;
}

zinline zerr_t zqueue_foreach(zqueue_t *que, zoperate op, zvalue_t hint){
    int pos = que->begin_pos;
    int end = 0;
    zchk_t *chunk = que->begin_chunk;
    if(!op || !que){
        return ZEPARAM_INVALID;
    }
    while(chunk){
        end = chunk == que->end_chunk ? que->end_pos : que->chunk_length;
        while(pos < end){
            op((zvalue_t)(chunk->value + pos), NULL, hint);
            pos += que->value_size;
        }
        chunk = chunk->next;
        pos = 0;
    }
    return ZEOK;
}

zinline int32_t zqueue_size(zqueue_t *que){
    return *que->size;
}

zinline zerr_t zqueue_back(zqueue_t *que, zvalue_t *pval){
    if(!*que->size){
        return ZENOT_EXIST;
    }
    if(que->end_pos){
        *pval = (zvalue_t)(que->end_chunk->value + que->end_pos - que->value_size);
    }else{
        *pval = (zvalue_t)(que->end_chunk->prev->value + que->chunk_length - que->value_size);
    }
    return ZEOK;
}

/**
 * @brief 0 memcopy push
 *
 * zvalue_t val;
 * zqueue_back_pos(que, &val);
 * init val...
 */
zinline zerr_t zqueue_back_pos(zqueue_t *que, zvalue_t *pval){
    *pval = (zvalue_t)(que->end_chunk->value + que->end_pos);
    que->end_pos += que->value_size;
    if(que->end_pos < que->chunk_length){
        zatm_inc(que->size);
        return ZEOK;
    }else{
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            return ZEMEM_INSUFFICIENT;
        }
        chunk->prev = que->end_chunk;
        chunk->next = NULL;
        que->end_chunk->next = chunk;
        que->end_chunk = chunk;
        que->end_pos = 0;
        zatm_inc(que->size);
    }
    return ZEOK;
}

zinline zerr_t zqueue_front(zqueue_t *que, zvalue_t *pval){
    if(!*que->size){
        return ZENOT_EXIST;
    }
    *pval = (zvalue_t)(que->begin_chunk->value + que->begin_pos);
    return ZEOK;
}

zinline void zqueue_swap(zqueue_t **que1, zqueue_t **que2){
    *que2 = (zqueue_t*)zatm_xchg_ptr(que1,*que2);
}

/*
 * mulit thread safe push/pop
 */
zinline zerr_t zqueue_lock_push(zqueue_t *que, zvalue_t value, zspinlock_t *spin){
    zspin_lock(spin);
    /*if(que->end_pos < que->chunk_size){*/
    memcpy(que->end_chunk->value + que->end_pos, value, que->value_size);
    que->end_pos += que->value_size;
    if(que->end_pos < que->chunk_length){
        zatm_inc(que->size);
        zspin_unlock(spin);
        //printf("lock_push<que:%p, size:%d>\n", que, *que->size);
        return ZEOK;
    }else{
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            zspin_unlock(spin);
            return ZEMEM_INSUFFICIENT;
        }
        chunk->prev = que->end_chunk;
        chunk->next = NULL;
        que->end_chunk->next = chunk;
        que->end_chunk = chunk;
        que->end_pos = 0;
        zatm_inc(que->size);
        zspin_unlock(spin);
    }
    return ZEOK;
}

zinline zerr_t zqueue_lock_pop(zqueue_t *que, zvalue_t *pval, zspinlock_t *spin){
    zspin_lock(spin);
    if(!*que->size){
        zspin_unlock(spin);
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    if(pval){
        *pval = (zvalue_t)(que->begin_chunk->value + que->begin_pos);
        //printf("lock_pop<que:%p, size:%d>\n", que, *que->size);
    }
    que->begin_pos += que->value_size;
    if(que->begin_pos == que->chunk_length){
        zchk_t *chunk = que->begin_chunk;
        que->begin_chunk = que->begin_chunk->next;
        que->begin_chunk->prev = NULL;
        que->begin_pos = 0;
        chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, chunk);
        zmem_align_free(chunk);
    }
    zspin_unlock(spin);
    return ZEOK;
}

zinline zerr_t zqueue_lock_push_front(zqueue_t *que, zvalue_t value, zspinlock_t *spin){
    zspin_lock(spin);
    que->begin_pos -= que->value_size;
    if(que->begin_pos < 0){
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            zspin_unlock(spin);
            return ZEMEM_INSUFFICIENT;
        }
        chunk->next = que->begin_chunk;
        chunk->prev = NULL;
        que->begin_chunk->prev = chunk;
        que->begin_chunk = chunk;
        que->begin_pos = que->chunk_length - que->value_size;
    }
    memcpy(que->begin_chunk->value + que->begin_pos, value, que->value_size);
    zatm_inc(que->size);
    zspin_unlock(spin);
    return ZEOK;
}

zinline zerr_t zqueue_lock_pop_back(zqueue_t *que, zvalue_t *pval, zspinlock_t *spin){
    zspin_lock(spin);
    if(!*que->size){
        zspin_unlock(spin);
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    que->end_pos -= que->value_size;
    if(que->end_pos < 0){
        zchk_t *chunk = que->end_chunk;
        que->end_chunk = que->end_chunk->prev;
        que->end_chunk->next = NULL;
        que->end_pos = que->chunk_length - que->value_size;
        chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, chunk);
        zmem_align_free(chunk);
    }
    if(pval){
        *pval = (zvalue_t)(que->end_chunk->value + que->end_pos);
    }
    zspin_unlock(spin);
    return ZEOK;
}

/**
 * @brief 0 memcopy push
 *
 * @par useage
 *      0 memcopy push
 *      zvalue_t val;
 *      if(ZOK == (ret = zqueue_lock_back_pos(que, &val, spin))){
 *          do something with val...
 *          zqueue_unlock_lock_back_pos(que, spin);
 *      }
 *
 *      0 memory no lock pop
 *      if(ZOK == zqueue_front(que, &val)){
 *          do something with val...
 *          zqueue_pop(que, NULL);
 *      }
 *
 *      0 memory locked pop
 *      if(ZOK == zqueue_lock_front(que, &val)){
 *          do something with val...
 *          zqueue_unlock_front(que, &val);
 *      }
 */
zinline zerr_t zqueue_lock_back_pos(zqueue_t *que, zvalue_t *pval,
                                    zspinlock_t *spin){
    zspin_lock(spin);
    *pval = (zvalue_t)(que->end_chunk->value + que->end_pos);
    que->end_pos += que->value_size;
    if(que->end_pos < que->chunk_length){
        /* current chunk has enough space */
        return ZEOK;
    }else{
        /* allocate new chunk */
        zchk_t *chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOCX(que->chunk_length);
        }
        if(!chunk){
            zspin_unlock(spin);
            return ZEMEM_INSUFFICIENT;
        }
        chunk->prev = que->end_chunk;
        chunk->next = NULL;
        que->end_chunk->next = chunk;
        que->end_chunk = chunk;
        que->end_pos = 0;
    }
    return ZEOK;
}

zinline void zqueue_unlock_back_pos(zqueue_t *que, zspinlock_t *spin){
    zatm_inc(que->size);
    zspin_unlock(spin);
}

zinline zerr_t zqueue_lock_front(zqueue_t *que, zvalue_t *pval,
                                 zspinlock_t *spin){
    zspin_lock(spin);
    if(!*que->size){
        zspin_unlock(spin);
        return ZENOT_EXIST;
    }
    *pval = (zvalue_t)(que->begin_chunk->value + que->begin_pos);
    return ZEOK;
}

zinline void zqueue_unlock_front(zqueue_t *que, zspinlock_t *spin){
    zatm_dec(que->size);
    que->begin_pos += que->value_size;
    if(que->begin_pos == que->chunk_length){
        zchk_t *chunk = que->begin_chunk;
        que->begin_chunk = que->begin_chunk->next;
        que->begin_chunk->prev = NULL;
        que->begin_pos = 0;
        chunk = (zchk_t*) zatm_xchg_ptr(&que->spare_chunk, chunk);
        zmem_align_free(chunk);
    }
    zspin_unlock(spin);
}
ZC_END

#endif
