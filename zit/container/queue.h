#ifndef _ZCONT_QUE_1R1W_H_
#define _ZCONT_QUE_1R1W_H_
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
 * @brief An efficient queue implementation. ZeroMQ yqueue like.
 * @details
 *  - (NOT) thread safe for multithread push()/pop()
 *  - minimize number of allocations/deallocations needded.
 *  - support 1 thread push() and 1 thread pop() with no lock,
 *    NOT SUPPORT 1 thread push() and 1 thread pop_front() with no lock
 *    NOT SUPPORT 1 thread push_front() and 1 thread pop() with no lock
 *  - for_each() only access by 1 thread;
 *  - inline implement
 *  - It's about 20000000 by 1 push thread and 1 pop thread per second, on (vm)ubuntu17
 *    Try ($ make/release/zit_test queue_1r1w) interval:0.499194 10 million
 *    Contrast zpin+std::queue with queue_1r1w push/pop 10 million;
 *    Try ($ make/release/zpp_test queue_1r1w) interval:1.036969 10 million
 *  - use inline module to improve 50% performance.
 */
#include <zit/base/type.h>

ZC_BEGIN

ZAPI zerr_t zque1_create(zcontainer_t *cont, void *hint);
ZAPI zerr_t zque1_destroy(zcontainer_t cont);
#if 0
ZAPI zerr_t zque1_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zque1_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zque1_popback(zcontainer_t cont, zvalue_t *out);
#endif
ZAPI zerr_t zque1_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zque1_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zque1_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zque1_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zque1_size(zcontainer_t cont);
ZAPI zerr_t zque1_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zque1_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zque1_swap(zcontainer_t *cont1, zcontainer_t *cont2);

/*
 * inline implements
 */
#include <zit/base/atomic.h>
#include <zit/base/error.h>

typedef struct zqueue_1r1w_s{
    zchunk_t *begin_chunk;
    zchunk_t *end_chunk;
    zchunk_t *spare_chunk;
    zatm_t size;
    int begin_pos;
    int end_pos;
    int back_pos;
    int chunk_size;
}zque1_t;

#define ZCHUNK_ALLOC(n) (zchunk_t*)zmem_align64(sizeof(zchunk_t) + (n)*sizeof(void*))

zinline zerr_t zque1_push(zcontainer_t cont, zvalue_t in){
    zque1_t *que = (zque1_t*)cont;
    zchunk_t *chunk  = NULL;
    if(que->end_pos < que->chunk_size){ // que->end_pos == que->chunk_size error status
        que->end_chunk->value[que->end_pos] = in;
    }
    if(++que->end_pos < que->chunk_size){
        zatm_inc(que->size);
        return ZOK;
    }
    chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
    if(!chunk){
        chunk = ZCHUNK_ALLOC(que->chunk_size);
        if(!chunk){
            return ZEMEM_INSUFFICIENT;
        }
    }

    chunk->prev = que->end_chunk;
    chunk->next = NULL;
    que->end_chunk->next = chunk;
    que->end_chunk = chunk;
    que->end_pos = 0;
    zatm_inc(que->size);
    return ZEOK;
}

zinline zerr_t zque1_pop(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    if(out){
        *out = que->begin_chunk->value[que->begin_pos];
    }
    if(++que->begin_pos == que->chunk_size){
        zchunk_t *chunk = que->begin_chunk;
        que->begin_chunk = que->begin_chunk->next;
        que->begin_chunk->prev = NULL;
        que->begin_pos = 0;
        chunk = zatm_xchg_ptr(&que->spare_chunk, chunk);
		zmem_align_free(chunk);
    }
    return ZEOK;
}

zinline zerr_t zque1_popback(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    --que->end_pos;
    if(que->end_pos < 0){
        zchunk_t *chunk = que->end_chunk;
        que->end_chunk = que->end_chunk->prev;
        que->end_chunk->next = NULL;
        que->end_pos = que->chunk_size - 1;
        chunk = zatm_xchg_ptr(&que->spare_chunk, chunk);
		zmem_align_free(chunk);
    }
    if(out){
        *out = que->end_chunk->value[que->end_pos];
    }
    return ZOK;
}

ZC_END

#endif
