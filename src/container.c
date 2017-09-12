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
#include <zit/container/queue.h>
#include <zit/base/atomic.h>
#include <zit/base/error.h>
#include <string.h>

#define ZTRACE_QUE1 0 // 0:not trace 1:trace
#define ZCHUNK_ALLOC(n) (zchunk_t*)zmem_align64(sizeof(zchunk_t) + (n)*sizeof(void*))

zerr_t zque1_create(zcontainer_t *cont, void *hint){
    zchunk_t *chunk;
    zque1_t *que = (zque1_t*)zmem_align64(sizeof(zque1_t));
    zatm_t size = (zatm_t)zatm_alloc_atm();
    int chunk_size = hint ? *((int*)hint) : ZCHUNK_SIZE;

    chunk = ZCHUNK_ALLOC(chunk_size);

    if(chunk && que && size){
        *cont = (zcontainer_t*)que;
        memset(que, 0, sizeof(zque1_t));
        chunk->next = chunk->prev = NULL;
        que->begin_chunk = que->end_chunk = chunk;
        que->size = size;
        que->chunk_size = chunk_size;
        *size = 0;
    }else{
        zmem_align_free(chunk);
		zmem_align_free(que);
        zatm_free(size);
        *cont = NULL;
        return ZEMEM_INSUFFICIENT;
    }
    return ZEOK;
}

zerr_t zque1_destroy(zcontainer_t cont){
    zque1_t *que = (zque1_t*)cont;
    zchunk_t *chunk;
    if(!que){
        return ZEOK;
    }
    chunk = que->begin_chunk;
    while(chunk != que->end_chunk){
        que->begin_chunk = que->begin_chunk->next;
#if ZTRACE_QUE1
        zprint("zmem_align_free chunk<%p>\n", chunk);
#endif
		zmem_align_free(chunk);
        chunk = que->begin_chunk;
    }
#if ZTRACE_QUE1
        zprint("zmem_align_free chunk<%p>\n", chunk);
#endif
		zmem_align_free(chunk);
    chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
#if ZTRACE_QUE1
    zprint("zmem_align_free spare chunk<%p>\ndestroy down\n", chunk);
#endif
	zmem_align_free(chunk);
	zatm_free(que->size);
	zmem_align_free(cont);
    return ZEOK;
}
zerr_t zque1_pushfront(zcontainer_t cont, zvalue_t in){
    zque1_t *que = (zque1_t*)cont;
    --que->begin_pos;
    if(que->begin_pos < 0){
        zchunk_t *chunk  = NULL;
        chunk = zatm_xchg_ptr(&que->spare_chunk, NULL);
        if(!chunk){
            chunk = ZCHUNK_ALLOC(que->chunk_size);
            if(!chunk){
                return ZEMEM_INSUFFICIENT;
            }
        }
        chunk->next = que->begin_chunk;
        chunk->prev = NULL;
        que->begin_chunk->prev = chunk;
        que->begin_chunk = chunk;
        que->begin_pos = que->chunk_size - 1;
#if ZTRACE_QUE1
        zprint("XCHG");
#endif

    }
    que->begin_chunk->value[que->begin_pos] = in;
    zatm_inc(que->size);
    return ZEOK;
}

zerr_t zque1_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    return ZENOT_SUPPORT;
}

zerr_t zque1_erase(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    return ZENOT_SUPPORT;
}
/**
 * @brief access each element by user define operater
 * @attention queue can only access by one thread, mulithread will cause error;
 */
zerr_t zque1_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zque1_t *que = (zque1_t*)cont;
    int pos = que->begin_pos;
    int end = 0;
    zchunk_t *chunk = que->begin_chunk;
    if(!op || !cont){
        return ZEPARAM_INVALID;
    }
    while(chunk){
        end = chunk == que->end_chunk ? que->end_pos : que->chunk_size;
        while(pos < end){
            op(chunk->value[pos++], NULL, hint);
        }
        chunk = chunk->next;
        pos = 0;
    }
    return ZOK;
}

size_t zque1_size(zcontainer_t cont){
    return *((zque1_t*)cont)->size;
}

zerr_t zque1_back(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    if(que->end_pos){
        *out = que->end_chunk->value[que->end_pos - 1];
    }else{
        *out = que->end_chunk->prev->value[que->chunk_size - 1];
    }
    return ZEOK;
}
zerr_t zque1_front(zcontainer_t cont, zvalue_t *out){
    zque1_t *que = (zque1_t*)cont;
    if(!*que->size){
        return ZENOT_EXIST;
    }
    *out = que->begin_chunk->value[que->begin_pos];
    return ZEOK;
}

zerr_t zque1_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    return ZEOK;
}

/*============================================================================*/
/*
 * implement queue_mt
 */
#include <zit/thread/spin.h>

typedef struct zqueue_nrnw_s{
    zcontainer_t cont;
    zspinlock_t front_spin;
    zspinlock_t back_spin;
}zquen_t;

zerr_t zquen_create(zcontainer_t *cont, void *hint){
    zerr_t ret;
    zquen_t *que = (zquen_t*)zmem_align64(sizeof(zquen_t));
    do{
        if(!que){
            ret = ZEMEM_INSUFFICIENT;
            break;
        }
        if(ZOK != (ret = zque1_create(&que->cont, hint)))break;
        zspin_init(&que->front_spin);
        zspin_init(&que->back_spin);
        *cont = que;
    }while(0);
    return ret;
}

zerr_t zquen_destroy(zcontainer_t cont){
    zquen_t *que;
    if(!cont){
        return ZEOK;
    }
    que = (zquen_t*)cont;
    zque1_destroy(que->cont);
    zspin_fini(&que->front_spin);
    zspin_fini(&que->back_spin);
    zmem_align_free(cont);
    return ZEOK;
}

zerr_t zquen_push(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_push(que->cont, in);
    zspin_unlock(&que->back_spin);
    return ret;
}

zerr_t zquen_pop(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_pop(que->cont, out);
    zspin_unlock(&que->front_spin);
    return ret;
}

zerr_t zquen_pushfront(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_pushfront(que->cont, in);
    zspin_unlock(&que->front_spin);
    return ret;
}

zerr_t zquen_popback(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_popback(que->cont, out);
    zspin_unlock(&que->back_spin);
    return ret;
}

zerr_t zquen_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_insert(que->cont, in, compare, condition);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

zerr_t zquen_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_erase(que->cont, hint, compare, condition);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

zerr_t zquen_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_foreach(que->cont, op, hint);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

size_t zquen_size(zcontainer_t cont){
    zquen_t *que = (zquen_t*)cont;
    return zque1_size(que->cont);
}

zerr_t zquen_back(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_back(que->cont, out);
    zspin_unlock(&que->back_spin);
    return ret;
}

zerr_t zquen_front(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_front(que->cont, out);
    zspin_unlock(&que->front_spin);
    return ret;
}

zerr_t zquen_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    zquen_t *que1 = (zquen_t*)cont1;
    zquen_t *que2 = (zquen_t*)cont2;
    zspin_lock(&que1->front_spin);
    zspin_lock(&que1->back_spin);
    zspin_lock(&que2->front_spin);
    zspin_lock(&que2->back_spin);
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    zspin_unlock(&que2->back_spin);
    zspin_unlock(&que2->front_spin);
    zspin_unlock(&que1->back_spin);
    zspin_unlock(&que1->front_spin);
    return ZEOK;
}

/*============================================================================*/
/*
 * implement list
 */
#include <zit/container/list.h>

typedef struct zlist_s{
    znod_t *head; ///< list head node
    znod_t *tail; ///< list tail node
    znod_t *recycle; ///< recycle buffer
    zatm_t size; ///< list size, 64bit aligned pointer.
    uint32_t rec_size; ///< recycle buffer size
    uint32_t rec_max_size; ///< max recycle buffer size, if > size just free buffer.
    zspinlock_t spin; ///< spin lock for recycle
}zlist_t;

zerr_t zlist_popnode(zlist_t *list, znod_t **nod);
zerr_t zlist_pushnode(zlist_t *list, znod_t *nod);
zerr_t zlist_freenode(znod_t *nod);

zerr_t zlist_popnode(zlist_t *list,  znod_t **nod){
    // ignore assert args
    zspin_lock(&list->spin);
    *nod = list->recycle;
    if(*nod){
        list->recycle = list->recycle->prev;
        --list->rec_size;
        zspin_unlock(&list->spin);
    }else{
        zspin_unlock(&list->spin);
        *nod = (znod_t*)zmem_align64(sizeof(znod_t));
    }
    return (*nod) ? ZEOK : ZEMEM_INSUFFICIENT;
}
zerr_t zlist_pushnode(zlist_t *list,  znod_t *nod){
    zspin_lock(&list->spin);
    if(list->rec_size < list->rec_max_size){
        nod->next = NULL;
        nod->prev = list->recycle;
        list->recycle = nod;
        ++list->rec_size;
        zspin_unlock(&list->spin);
    }else{
        zspin_unlock(&list->spin);
        zmem_align_free(nod);
    }
    return ZEOK;
}
zerr_t zlist_freenode(znod_t *back){
    znod_t *nod = back;
    while(nod){
        back = back->prev;
        zmem_align_free(nod);
        nod = back;
    }
    return ZEOK;
}

/**
 * @fn zerr_t zlist_create(zcontainer_t *cont)
 * @brief create a list
 * @note
 *  list{head, tail, ...}
 *         |     \------------\
 *       node{               node{
 *         value:dummy         value:dummy
 *  NULL<--prev  <-----------  prev
 *         next  ----------->  next  -------->NULL
 *       }                    }
 */
zerr_t zlist_create(zcontainer_t *cont, zvalue_t hint){
    zlist_t *list;
    zatm_t size;
    znod_t *head;
    znod_t *tail;

    size = (zatm_t)zatm_alloc_atm();
    *cont = zmem_align64(sizeof(zlist_t));
    head = (znod_t*)zmem_align64(sizeof(znod_t));
    tail = (znod_t*)zmem_align64(sizeof(znod_t));

    if(!*cont || !size || !head || !tail){
        zmem_align_free(*cont);
        zatm_free(size);
        zmem_align_free(head);
        zmem_align_free(tail);
        return ZEMEM_INSUFFICIENT;
    }
    list = (zlist_t*)(*cont);
    memset(list, 0, sizeof(zlist_t));
    list->rec_max_size = 4096;
    *size = 0;
    list->size = size;
    zspin_init(&list->spin);

    list->head = head;
    list->tail = tail;

    head->prev = NULL;
    head->next = tail;

    tail->prev = head;
    tail->next = NULL;
    return ZOK;
}

zerr_t zlist_destroy(zcontainer_t cont){
    zlist_t *list = (zlist_t*)cont;
    if(!cont){
        return ZEOK;
    }
    zlist_freenode(list->recycle);
    zlist_freenode(list->tail);
    zspin_fini(&list->spin);
    zatm_free(list->size);
    zmem_align_free(list);
    return ZOK;
}

zerr_t zlist_push(zcontainer_t cont, zvalue_t in){
    zlist_t *list;
    znod_t *nod;
    znod_t *tnod;
    zerr_t ret;

    list = (zlist_t*)cont;
    if(ZOK != (ret = zlist_popnode(list, &nod))){
        return ret;
    }

    tnod = list->tail;
    tnod->value = in;
    tnod->next = nod;

    nod->prev = tnod;
    nod->next = NULL;

    list->tail = nod;

    zatm_inc(list->size); // increment list size
    return ZOK;
}
zerr_t zlist_pop(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->head;

    if(!*list->size){
        return ZENOT_EXIST;
    }
    zatm_dec(list->size); // decrement list size

    *out = nod->next->value;
    list->head = nod->next;
    list->head->prev = NULL;

    zlist_pushnode(list, nod);
    return ZEOK;
}

zerr_t zlist_pushfront(zcontainer_t cont, zvalue_t in){
    zlist_t *list;
    znod_t *nod;
    znod_t *tnod;
    zerr_t ret;

    list = (zlist_t*)cont;
    if(ZOK != (ret = zlist_popnode(list, &nod))){
        return ret;
    }

    tnod = list->head;
    tnod->value = in;
    tnod->prev = nod;

    nod->next = tnod;
    nod->prev = NULL;

    list->head = nod;

    zatm_inc(list->size); // increment list size
    return ZEOK;
}

zerr_t zlist_popback(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->tail;

    if(!*list->size){
        return ZENOT_EXIST;
    }
    zatm_dec(list->size); // decrement list size

    *out = nod->prev->value;
    list->tail = nod->prev;
    list->tail->next = NULL;

    zlist_pushnode(list, nod);
    return ZEOK;
}

zerr_t zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    // commit single thread access
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->head;
    znod_t *in_nod = NULL;
    int cmp = ZEQUAL;
    zerr_t ret = ZEOK;

    while(nod->next && nod->next->next){
        if(ZEOK == compare(nod->next->value, (zvalue_t*)&cmp, in) && cmp == condition){
            if(ZEOK != (ret = zlist_popnode(list, &in_nod))){
                break;
            }
            in_nod->value = in;

            nod->next->prev = in_nod;
            in_nod->next = nod->next;

            in_nod->prev = nod;
            nod->next = in_nod;
            zatm_inc(list->size);
            break;
        }
        nod = nod->next;
    }

    return ret;
}

zerr_t zlist_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->head;
    znod_t *in_nod = NULL;
    int cmp = ZEQUAL;
    zerr_t ret = ZEOK;

    while(nod->next && nod->next->next){
        if(ZEOK == compare(nod->next->value, (zvalue_t*)&cmp, hint) && cmp == condition){
            zatm_dec(list->size);
            in_nod = nod->next->next;
            zlist_pushnode(list, nod->next);

            in_nod->prev = nod;
            nod->next = in_nod;
            continue;
        }
        nod = nod->next;
    }

    return ret;
}

zerr_t zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->head;

    while(nod->next && nod->next->next){
        op(nod->next->value, NULL, hint);
        nod = nod->next;
    }
    return ZEOK;
}

size_t zlist_size(zcontainer_t cont){
    return *((zlist_t*)cont)->size;
}
zerr_t zlist_back(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    if(*list->size){
        *out = list->tail->prev->value;
        return ZEOK;
    }
    return ZENOT_EXIST;
}
zerr_t zlist_front(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    if(*list->size){
        *out = list->head->next->value;
        return ZEOK;
    }
    return ZENOT_EXIST;
}

zerr_t zlist_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    return ZEOK;
}

/*============================================================================*/
/*
 * implement list_mt
 */
#include <zit/container/list_mt.h>

typedef struct zlist_mt_s{
    zcontainer_t cont;
    zspinlock_t front_spin;
    zspinlock_t back_spin;
}zlist_mt_t;

zerr_t zlist_mt_create(zcontainer_t *cont, zvalue_t hint){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)zmem_align64(sizeof(zlist_mt_t));
    do{
        if(!list){
            ret = ZEMEM_INSUFFICIENT;
            break;
        }
        if(ZOK != (ret = zlist_create(&list->cont, hint)))break;
        zspin_init(&list->front_spin);
        zspin_init(&list->back_spin);
        *cont = list;
    }while(0);
    return ret;
}

zerr_t zlist_mt_destroy(zcontainer_t cont){
    zlist_mt_t *list;
    if(!cont){
        return ZEOK;
    }
    list = (zlist_mt_t*)cont;
    zlist_destroy(list->cont);
    zspin_fini(&list->front_spin);
    zspin_fini(&list->back_spin);
    zmem_align_free(cont);
    return ZEOK;
}

zerr_t zlist_mt_push(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_push(list->cont, in);
    zspin_unlock(&list->back_spin);
    return ret;
}

zerr_t zlist_mt_pop(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_pop(list->cont, out);
    zspin_unlock(&list->front_spin);
    return ret;
}

zerr_t zlist_mt_pushfront(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_pushfront(list->cont, in);
    zspin_unlock(&list->front_spin);
    return ret;
}

zerr_t zlist_mt_popback(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_popback(list->cont, out);
    zspin_unlock(&list->back_spin);
    return ret;
}

zerr_t zlist_mt_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_insert(list->cont, in, compare, condition);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

zerr_t zlist_mt_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_erase(list->cont, hint, compare, condition);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

zerr_t zlist_mt_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_foreach(list->cont, op, hint);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

size_t zlist_mt_size(zcontainer_t cont){
    zlist_mt_t *list = (zlist_mt_t*)cont;
    return zlist_size(list->cont);
}

zerr_t zlist_mt_back(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_back(list->cont, out);
    zspin_unlock(&list->back_spin);
    return ret;
}

zerr_t zlist_mt_front(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_front(list->cont, out);
    zspin_unlock(&list->front_spin);
    return ret;
}

zerr_t zlist_mt_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    zlist_mt_t *list1 = (zlist_mt_t*)cont1;
    zlist_mt_t *list2 = (zlist_mt_t*)cont2;
    zspin_lock(&list1->front_spin);
    zspin_lock(&list1->back_spin);
    zspin_lock(&list2->front_spin);
    zspin_lock(&list2->back_spin);
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    zspin_unlock(&list2->back_spin);
    zspin_unlock(&list2->front_spin);
    zspin_unlock(&list1->back_spin);
    zspin_unlock(&list1->front_spin);
    return ZEOK;
}

/*============================================================================*/
/*
 * implement container
 */
#include <zit/container/container.h>

typedef zerr_t (*zcont_destroy)(zcontainer_t cont);
typedef zerr_t (*zcont_push)(zcontainer_t cont, zvalue_t in);
typedef zerr_t (*zcont_pop)(zcontainer_t cont, zvalue_t *out);
typedef zerr_t (*zcont_pushfront)(zcontainer_t cont, zvalue_t in);
typedef zerr_t (*zcont_popback)(zcontainer_t cont, zvalue_t *out);
typedef zerr_t (*zcont_insert)(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
typedef zerr_t (*zcont_erase)(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
typedef zerr_t (*zcont_foreach)(zcontainer_t cont, zoperate op, zvalue_t hint);
typedef size_t (*zcont_size)(zcontainer_t cont);
typedef zerr_t (*zcont_back)(zcontainer_t cont, zvalue_t *out);
typedef zerr_t (*zcont_front)(zcontainer_t cont, zvalue_t *out);

typedef struct zcontainer_s{
    zcontainer_t cont;
    zcont_destroy destroy;
    zcont_push push;
    zcont_pop pop;
    zcont_pushfront pushfront;
    zcont_popback popback;
    zcont_insert insert;
    zcont_erase erase;
    zcont_foreach foreach;
    zcont_size size;
    zcont_back back;
    zcont_front front;
    int type;
}zcont_t;

zerr_t zcontainer_create(zcontainer_t *cont, int type, void *hint){
    zerr_t ret = ZEOK;
    zcont_t *cnt;
    ZASSERT(!cont);
    *cont = zmem_align64(sizeof(zcont_t));

    if(!*cont){
        return ZEMEM_INSUFFICIENT;
    }

    cnt = (zcont_t*)(*cont);
    cnt->type = type;
    switch(type){
    case ZCONT_TYPE_LIST:
        ret = zlist_create(&cnt->cont, hint);
        if(ZOK == ret){
            cnt->destroy = zlist_destroy;
            cnt->push = zlist_push;
            cnt->pop = zlist_pop;
            cnt->pushfront = zlist_pushfront;
            cnt->popback = zlist_popback;
            cnt->insert = zlist_insert;
            cnt->erase = zlist_erase;
            cnt->foreach = zlist_foreach;
            cnt->size = zlist_size;
            cnt->back = zlist_back;
            cnt->front = zlist_front;
        }
        break;
    case ZCONT_TYPE_QUEUE:
        ret = zque1_create(&cnt->cont, hint);
        if(ZOK == ret){
            cnt->destroy = zque1_destroy;
            cnt->push = zque1_push;
            cnt->pop = zque1_pop;
            cnt->pushfront = zque1_pushfront;
            cnt->popback = zque1_popback;
            cnt->insert = zque1_insert;
            cnt->erase = zque1_erase;
            cnt->foreach = zque1_foreach;
            cnt->size = zque1_size;
            cnt->back = zque1_back;
            cnt->front = zque1_front;
        }
        break;
    case ZCONT_TYPE_QUE_MT:
        ret = zquen_create(&cnt->cont, hint);
        if(ZOK == ret){
            cnt->destroy = zquen_destroy;
            cnt->push = zquen_push;
            cnt->pop = zquen_pop;
            cnt->pushfront = zquen_pushfront;
            cnt->popback = zquen_popback;
            cnt->insert = zquen_insert;
            cnt->erase = zquen_erase;
            cnt->foreach = zquen_foreach;
            cnt->size = zquen_size;
            cnt->back = zquen_back;
            cnt->front = zquen_front;
        }
        break;
    case ZCONT_TYPE_LIST_MT:
        ret = zlist_mt_create(&cnt->cont, hint);
        if(ZOK == ret){
            cnt->destroy = zlist_mt_destroy;
            cnt->push = zlist_mt_push;
            cnt->pop = zlist_mt_pop;
            cnt->pushfront = zlist_mt_pushfront;
            cnt->popback = zlist_mt_popback;
            cnt->insert = zlist_mt_insert;
            cnt->erase = zlist_mt_erase;
            cnt->foreach = zlist_mt_foreach;
            cnt->size = zlist_mt_size;
            cnt->back = zlist_mt_back;
            cnt->front = zlist_mt_front;
        }
        break;
    default:
        ret = ZENOT_SUPPORT;
    }
    if(ZOK != ret){
        zmem_align_free(cnt);
        *cont = NULL;
    }
    return ret;
}
zerr_t zcontainer_destroy(zcontainer_t cont){
    zcont_t *cnt = (zcont_t*)cont;
    if(!cont){
        return ZEOK;
    }
    cnt->destroy(cnt->cont);
	zmem_align_free(cnt);
	return ZEOK;
}

zerr_t zcontainer_push(zcontainer_t cont, zvalue_t in){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->push(cnt->cont, in));
}
zerr_t zcontainer_pop(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->pop(cnt->cont, out));
}
zerr_t zcontainer_pushfront(zcontainer_t cont, zvalue_t in){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->pushfront(cnt->cont, in));
}
zerr_t zcontainer_popback(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->popback(cnt->cont, out));
}
zerr_t zcontainer_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->insert(cnt->cont, in, compare, condition));
}
zerr_t zcontainer_erase(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->erase(cnt->cont, in, compare, condition));
}
zerr_t zcontainer_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->foreach(cnt->cont, op, hint));
}

size_t zcontainer_size(zcontainer_t cont){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->size(cnt->cont));
}

zerr_t zcontainer_back(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->back(cnt->cont, out));
}
zerr_t zcontainer_front(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->front(cnt->cont, out));
}

zerr_t zcontainer_swap(zcontainer_t cont1, zcontainer_t cont2){
    //zcont_t *cnt = (zcont_t*)cont;
    //return(cnt->swap(cont1, cont2));
    return ZEOK;
}

/*
 * implements red-black tree
 */
#include <zit/container/base/rbtree.h>
#include <zit/base/trace.h>
zerr_t zrbtree_create(zbtree_t **tree, int32_t node_size, int32_t capacity,
                      zoperate compare, int is_mutiple){
    int ret = ZEOK;
    zalloc_t *alc = NULL;
    zbtree_t *tr = (zbtree_t*)calloc(1, sizeof(zbtree_t));
    zalloc_create(&alc, node_size, capacity);
    if(tr && alc){
        tr->root = NULL;
        tr->alloc = alc;
        tr->data_size = node_size - (int32_t)sizeof(zbtnode_t);
        tr->cmp = compare;
        tr->multiple = is_mutiple;
        *tree = tr;
        zspin_init(&tr->spin);
    }else{
        free(tr);
        zalloc_destroy(alc);
    }
    return ret;
}

zerr_t zrbtree_destroy(zbtree_t *tree){
    if(tree){
        zspin_fini(&tree->spin);
        zalloc_destroy(tree->alloc);
        free(tree);
    }
    return ZEOK;
}

void zrbtree_printx(zbtree_t *tree, zoperate format){
    zbtnode_t *refer = tree->root;
    zbtnode_t *parent = NULL;
    zbtnode_t *tmp_nod = NULL;
    int level = 0;
    int i = 0;
    char buf[512];
    const char *prefix_line[128] = {0};
    const char *arrow_red = "---> ";
    const char *arrow_blk = "-->> ";
    const char *arrow_space = "     ";

    const char *left_flag = "  |";
    const char *left_arrow = "  \\";
    const char *left_space = "   ";

    printf("\n red-black tree:\n");
    prefix_line[level] = NULL;
    if(!refer){
        printf("tree is empty.\n");
        return;
    }
    if(zrbn_is_red(refer)){
        printf("ERROR: root is not black node.\n");
    }
    zspin_lock(&tree->spin);
    for(;;){
        if(level == 0 && prefix_line[level] == left_space){
            break; /* print down */
        }
        if(format){
            tmp_nod = refer;
            buf[0] = 0;
            while(tmp_nod){
                format((zvalue_t)tmp_nod->data, NULL, buf + strlen(buf));
                tmp_nod = tmp_nod->next;
            }
            printf("%s", buf);
        }else{
            printf("%03d", zrbn_key(refer));
        }
        if(refer->left){
            /* has left sub tree , set left flag*/
            prefix_line[level] = left_flag;
        }else{
            prefix_line[level] = left_space; /* No left tree, set down flag*/
        }

        while(refer->right){
            if(level > 124){
                printf("\n WARNING:\n Too much level, can not print more.\n");
                zspin_unlock(&tree->spin);
                return;
            }
            /* Forward right to end, print all right nodes*/
            parent = refer;
            refer = refer->right;
            if(refer->parent != parent){
                printf("\n******************* refer<parent:%p> != parent<%p> ******************\n", refer->parent, parent);
            }
            if(format){
                tmp_nod = refer;
                buf[0] = 0;
                while(tmp_nod){
                    format((zvalue_t)tmp_nod->data, NULL, buf + strlen(buf));
                    tmp_nod = tmp_nod->next;
                }
                printf("%s%s", zrbn_is_black(refer) ? arrow_blk : arrow_red, buf);
            }else{
                printf("%s%03d", zrbn_is_black(refer) ? arrow_blk : arrow_red, zrbn_key(refer));
            }
            /* make prefix line */
            prefix_line[++level] = arrow_space;
            prefix_line[++level] = refer->left ? left_flag : left_space;
        }
        printf("\n");
        for(i=0; i<=level; ++i){
            if(prefix_line[i] == left_arrow){
                prefix_line[i] = left_space;
            }else if(prefix_line[i] == arrow_red || prefix_line[i] == arrow_blk){
                prefix_line[i] = arrow_space;
            }
        }
        for(;;){
            /* look back upon left */
            if(prefix_line[level] == left_flag){
                parent = refer;
                refer = refer->left;
                if(refer->parent != parent){
                    printf("\n******************* refer<parent:%p> != parent<%p> ******************\n", refer->parent, parent);
                }
                prefix_line[level] = left_arrow;
                for(i=0; i<=level; ++i){
                    printf("%s", prefix_line[i]);
                }
                prefix_line[++level] = zrbn_is_black(refer) ? arrow_blk : arrow_red;
                printf("%s", prefix_line[level]);
                ++level;
                break;
            }
            level -= 2;
            refer = refer->parent;
            if(level < 0 || !refer){
                zspin_unlock(&tree->spin);
                return;
            }
        }
    }
    zspin_unlock(&tree->spin);
}

/**
 * @brief print a red-black tree
 *  0  1    2  3    4  5    6          level
 *  030-->> 070-->> 085---> 090
 *    |       |       \---> 080
 *    |       \---> 060 --> 065
 *    |               \-->> 050 --> 055
 *    |                       \---> 040
 *    \-->> 015 ->> 020
 *            \-->> 010
 *                    \---> 005
 */
void zrbtree_print(zbtree_t *tree){
    zbtnode_t *refer = tree->root;
    zbtnode_t *parent = NULL;
    int level = 0;
    int i = 0;
    const char *prefix_line[128] = {0};
    const char *arrow_red = "---> ";
    const char *arrow_blk = "-->> ";
    const char *arrow_space = "     ";

    const char *left_flag = "  |";
    const char *left_arrow = "  \\";
    const char *left_space = "   ";

    printf("\n red-black tree:\n");
    prefix_line[level] = NULL;
    if(!refer){
        printf("tree is empty.\n");
        return;
    }
    if(zrbn_is_red(refer)){
        printf("ERROR: root is not black node.\n");
    }
    zspin_lock(&tree->spin);
    for(;;){
        if(level == 0 && prefix_line[level] == left_space){
            break; /* print down */
        }
        printf("%03d", zrbn_key(refer));
        if(refer->left){
            /* has left sub tree , set left flag*/
            prefix_line[level] = left_flag;
        }else{
            prefix_line[level] = left_space; /* No left tree, set down flag*/
        }

        while(refer->right){
            if(level > 124){
                printf("\n WARNING:\n Too much level, can not print more.\n");
                zspin_unlock(&tree->spin);
                return;
            }
            /* Forward right to end, print all right nodes*/
            parent = refer;
            refer = refer->right;
            if(refer->parent != parent){
                printf("\n******************* refer<parent:%p> != parent<%p> ******************\n", refer->parent, parent);
            }
            printf("%s%03d", zrbn_is_black(refer) ? arrow_blk : arrow_red, zrbn_key(refer));
            /* make prefix line */
            prefix_line[++level] = arrow_space;
            prefix_line[++level] = refer->left ? left_flag : left_space;
        }
        printf("\n");
        for(i=0; i<=level; ++i){
            if(prefix_line[i] == left_arrow){
                prefix_line[i] = left_space;
            }else if(prefix_line[i] == arrow_red || prefix_line[i] == arrow_blk){
                prefix_line[i] = arrow_space;
            }
        }
        for(;;){
            /* look back upon left */
            if(prefix_line[level] == left_flag){
                parent = refer;
                refer = refer->left;
                if(refer->parent != parent){
                    printf("\n******************* refer<parent:%p> != parent<%p> ******************\n", refer->parent, parent);
                }
                prefix_line[level] = left_arrow;
                for(i=0; i<=level; ++i){
                    printf("%s", prefix_line[i]);
                }
                prefix_line[++level] = zrbn_is_black(refer) ? arrow_blk : arrow_red;
                printf("%s", prefix_line[level]);
                ++level;
                break;
            }
            level -= 2;
            refer = refer->parent;
            if(level < 0 || !refer){
                zspin_unlock(&tree->spin);
                return;
            }
        }
    }
    zspin_unlock(&tree->spin);
}

void zrbtree_foreach(zbtnode_t *root, int order, zoperate do_each, zvalue_t hint){
    zbtnode_t *refer = root;
    zbtnode_t *child = NULL;
    if(refer){
        if(order == 1){
            child = refer;
            while(child){
                do_each((zvalue_t)child->data, NULL, hint);
                child = child->next;
            }
        }
        zrbtree_foreach(refer->left, order, do_each, hint);
        if(order == 0){
            child = refer;
            while(child){
                do_each((zvalue_t)child->data, NULL, hint);
                child = child->next;
            }
        }
        zrbtree_foreach(refer->right, order, do_each, hint);
        if(order != 0 && order != 1){
            child = refer;
            while(child){
                do_each((zvalue_t)child->data, NULL, hint);
                child = child->next;
            }
        }
    }
}

/* red-black tree property:
 * 1. Each node is either red or black:
 * 2. The root node is black.
 * 3. All leaves (shown as NIL in the above diagram) are black and contain no
 *    data. Since we represent these empty leaves using NULL, this property is
 *    implicitly assured by always treating NULL as black.
 *    To this end we create a node_color() helper function:
 * 4. Every red node has two children, and both are black (or equivalently,
 *    the parent of every red node is black).
 * 5. All paths from any given node to its leaf nodes contain ithe same number
 *    of black node.
 */
static zerr_t zrbtree_property1(zbtnode_t *node){
    int ret = ZOK;
    if(!node){
        return ZOK;
    }
    if(zrbn_is_black(node) || zrbn_is_red(node)){
        if(ZOK == (ret = zrbtree_property1(node->left))){
            ret = zrbtree_property1(node->right);
        }
    }else{
        ret = ZPARAM_INVALID;
        ZERR("Node<key:%d> not black or red color.", zrbn_key(node));
    }
    return ret;
}

static zerr_t zrbtree_property2(zbtree_t *tree){
    return zrbn_is_black(tree->root) ? ZOK : ZPARAM_INVALID;
}

static zerr_t zrbtree_property4(zbtnode_t *node){
    int ret = ZPARAM_INVALID;
    do{
        if(!node){
            ret = ZOK;
            break;
        }
        if(zrbn_is_red(node)){
            if(!zrbn_is_black(node->left)){
                break;
            }
            if(!zrbn_is_black(node->right)){
                break;
            }
            if(!zrbn_is_black(node->parent)){
                break;
            }
        }
        if(ZOK != (ret = zrbtree_property4(node->left))){
            break;
        }
        if(ZOK != (ret = zrbtree_property4(node->right))){
            break;
        }
        ret = ZOK;
    }while(0);
    return ret;
}

static zerr_t zrbtree_property5_helper(zbtnode_t *node, int black_count,
                                       int *path_black_count){
    int ret = ZOK;
    if(zrbn_is_black(node)){
        black_count++;
    }
    if(!node){
        if(-1 == *path_black_count){
            *path_black_count = black_count;
        }else{
            ret = (black_count == *path_black_count) ? ZOK : ZPARAM_INVALID;
        }
        return ret;
    }
    if(ZOK == (ret = zrbtree_property5_helper(node->left, black_count, path_black_count))){
        ret = zrbtree_property5_helper(node->right, black_count, path_black_count);
    }
    return ret;
}
static zerr_t zrbtree_property5(zbtnode_t *node){
    int black_count_path = -1;
    return zrbtree_property5_helper(node, 0, &black_count_path);
}

zerr_t zrbtree_verify(zbtree_t *tree){
    zerr_t ret = ZOK;
    do{
        if(ZOK != (ret = zrbtree_property1(tree->root))){
            break;
        }
        if(ZOK != (ret = zrbtree_property2(tree))){
            break;
        }
        if(ZOK != (ret = zrbtree_property4(tree->root))){
            break;
        }
        if(ZOK != (ret = zrbtree_property5(tree->root))){
            break;
        }
    }while(0);
    ZERRCX(ret);
    return ret;
}
