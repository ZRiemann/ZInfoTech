#ifndef _ZCONT_QUE_1R1W_H_
#define _ZCONT_QUE_1R1W_H_
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
 */
#include <zit/base/platform.h>
#include <zit/base/type.h>

zerr_t zque1_create(zcontainer_t *cont);
zerr_t zque1_destroy(zcontainer_t cont);
zerr_t zque1_push(zcontainer_t cont, zvalue_t in); // push back
zerr_t zque1_pop(zcontainer_t cont, zvalue_t *out); // pop front
zerr_t zque1_pushfront(zcontainer_t cont, zvalue_t in);
zerr_t zque1_popback(zcontainer_t cont, zvalue_t *out);
zerr_t zque1_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
zerr_t zque1_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
zerr_t zque1_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
zsize_t zque1_size(zcontainer_t cont);
zerr_t zque1_back(zcontainer_t cont, zvalue_t *out);
zerr_t zque1_front(zcontainer_t cont, zvalue_t *out);
zerr_t zque1_swap(zcontainer_t *cont1, zcontainer_t *cont2);

/*
 * inline implements
 */
#include <zit/base/atomic.h>
#include <zit/base/error.h>
#include <malloc.h>
#include <string.h> // memset()

#define ZTRACE_QUE1 0 // 0:not trace 1:trace

typedef struct zqueue_1r1w_s{
    zchunk_t *begin_chunk;
    zchunk_t *end_chunk;
    zchunk_t *spare_chunk;
    int *size;
    int begin_pos;
    int end_pos;
    int back_pos;
}zque1_t;

inline zerr_t zque1_create(zcontainer_t *cont){
    zchunk_t *chunk = (zchunk_t*)zmem_align64(sizeof(zchunk_t));
    zque1_t *que = (zque1_t*)zmem_align64(sizeof(zque1_t));
    int *size = (int*)zmem_align64(sizeof(int));
    if(chunk && que && size){
        *cont = (zcontainer_t*)que;
        memset(que, 0, sizeof(zque1_t));
        chunk->next = chunk->prev = NULL;
        que->begin_chunk = que->end_chunk = chunk;
        que->size = size;
        *size = 0;
    }else{
        free(chunk);
        free(que);
        free(size);
        *cont = NULL;
        return ZEMEM_INSUFFICIENT;
    }
    return ZEOK;
}

inline zerr_t zque1_destroy(zcontainer_t cont){
    zque1_t *que = (zque1_t*)cont;
    zchunk_t *chunk;
    if(!que){
        return ZEOK;
    }
    chunk = que->begin_chunk;
    while(chunk != que->end_chunk){
        que->begin_chunk = que->begin_chunk->next;
#if ZTRACE_QUE1
        zprint("free chunk<%p>\n", chunk);
#endif
        free(chunk);
        chunk = que->begin_chunk;
    }
#if ZTRACE_QUE1
        zprint("free chunk<%p>\n", chunk);
#endif
    free(chunk);
    chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
#if ZTRACE_QUE1
    zprint("free spare chunk<%p>\ndestroy down\n", chunk);
#endif
    free(chunk);
    free(cont);
    return ZEOK;
}

inline zerr_t zque1_push(zcontainer_t cont, zvalue_t in){
    zque1_t *que = (zque1_t*)cont;
    zchunk_t *chunk  = NULL;
    if(que->end_pos < ZCHUNK_SIZE){ // que->end_pos == ZCHUNK_SIZE error status
        que->end_chunk->value[que->end_pos] = in;
    }
    if(++que->end_pos < ZCHUNK_SIZE){
        zatm_inc(que->size);
        return ZOK;
    }
    chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
    if(!chunk){
        chunk = (zchunk_t*)zmem_align64(sizeof(zchunk_t));
#if ZTRACE_QUE1
        zprint("mem_align: %p", chunk);
#endif
        if(!chunk){
            return ZEMEM_INSUFFICIENT;
        }
    }
#if ZTRACE_QUE1
    else{
        zprint("EXCHG reuse: %p", chunk);
    }
#endif
    chunk->prev = que->end_chunk;
    chunk->next = NULL;
    que->end_chunk->next = chunk;
    que->end_chunk = chunk;
    que->end_pos = 0;
    zatm_inc(que->size);
    return ZEOK;
}

inline zerr_t zque1_pop(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    zatm_dec(que->size);
    *out = que->begin_chunk->value[que->begin_pos];
    if(++que->begin_pos == ZCHUNK_SIZE){
        zchunk_t *chunk = que->begin_chunk;
        que->begin_chunk = que->begin_chunk->next;
        que->begin_chunk->prev = NULL;
        que->begin_pos = 0;
        chunk = zatm_xchg_ptr(&que->spare_chunk, chunk);
#if ZTRACE_QUE1
        if(chunk){
            zprint("XCHG free: %p", chunk);
        }else{
            zprint("XCHG (NULL)");
        }
#endif
        free(chunk);
    }
    return ZEOK;
}

inline zerr_t zque1_pushfront(zcontainer_t cont, zvalue_t in){
    zque1_t *que = (zque1_t*)cont;
    --que->begin_pos;
    if(que->begin_pos < 0){
        zchunk_t *chunk  = NULL;
        chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = (zchunk_t*)zmem_align64(sizeof(zchunk_t));
            if(!chunk){
                return ZEMEM_INSUFFICIENT;
            }
        }
        chunk->next = que->begin_chunk;
        chunk->prev = NULL;
        que->begin_chunk->prev = chunk;
        que->begin_chunk = chunk;
        que->begin_pos = ZCHUNK_SIZE;
#if ZTRACE_QUE1
        zprint("XCHG");
#endif

    }
    que->begin_chunk->value[que->begin_pos] = in;
    zatm_inc(que->size);
    return ZEOK;
}
inline zerr_t zque1_popback(zcontainer_t cont, zvalue_t *out){
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
        que->end_pos = ZCHUNK_SIZE-1;
        chunk = zatm_xchg_ptr(&que->spare_chunk, chunk);
        free(chunk);
#if ZTRACE_QUE1
        zprint("XCHG free<%p>");
#endif

    }
    *out = que->end_chunk->value[que->end_pos];
    return ZOK;
}

inline zerr_t zque1_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    return ZENOT_SUPPORT;
}

inline zerr_t zque1_erase(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    return ZENOT_SUPPORT;
}
/**
 * @brief access each element by user define operater
 * @attention queue can only access by one thread, mulithread will cause error;
 */
inline zerr_t zque1_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zque1_t *que = (zque1_t*)cont;
    int pos = que->begin_pos;
    int end = 0;
    zchunk_t *chunk = que->begin_chunk;
    if(!op || !cont){
        return ZEPARAM_INVALID;
    }
    while(chunk){
        end = chunk == que->end_chunk ? que->end_pos : ZCHUNK_SIZE;
        while(pos < end){
            op(chunk->value[pos++], NULL, hint);
        }
        chunk = chunk->next;
        pos = 0;
    }
    return ZOK;
}

inline zsize_t zque1_size(zcontainer_t cont){
    return *((zque1_t*)cont)->size;
}

inline zerr_t zque1_back(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    if(que->end_pos){
        *out = que->end_chunk->value[que->end_pos-1];
    }else{
        *out = que->end_chunk->prev->value[ZCHUNK_SIZE-1];
    }
    return ZEOK;
}
inline zerr_t zque1_front(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    *out = que->begin_chunk->value[que->begin_pos];
    return ZEOK;
}

inline zerr_t zque1_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    return ZEOK;
}

#endif
