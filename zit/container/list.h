#ifndef _ZCONT_LIST_H_
#define _ZCONT_LIST_H_
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
 * @file zit/container/list.h
 * @brief A doubly linked list
 */
#include <zit/base/type.h>
#include <zit/base/error.h>
#include <zit/base/atomic.h>
#include <zit/memory/alloc.h>
#include <string.h>

ZC_BEGIN

/**
 * @brief list node structure
 */
typedef struct znode_s{
    struct znode_s *prev; /** previous node pointer */
    struct znode_s *next; /** next node pointer */
    char data[]; /** user define data */
}znod_t;

typedef struct zlist_s{
    znod_t *head; /** list head node */
    znod_t *tail; /** list tail node */
    zatm_t size; /** list size, 64bit aligned pointer */
    zalloc_t *alloc; /** memory allocator */
    int data_size; /** user data size */
}zlist_t;

zinline zerr_t zlist_create(zlist_t **list, int32_t capacity,
                            int32_t data_size){
    int ret = ZEMEM_INSUFFICIENT;
    zalloc_t *alc = NULL;
    zlist_t *lst = (zlist_t*)calloc(1, sizeof(zlist_t));
    do{
        if(!lst){
            break;
        }

        ret = zalloc_create(&alc, (int32_t)sizeof(znod_t) + data_size,
                            capacity, 512 * 1024 *1024);
        if(ZEOK != ret){
            break;
        }

        lst->size = zatm_alloc_atm();
        if(!lst->size){
            break;
        }
        *lst->size = 0;
        lst->alloc = alc;
        lst->data_size = data_size;

        *list = lst;
        ret = ZEOK;
    }while(0);

    if(ZEOK != ret){
        free(lst);
        zalloc_destroy(alc);
    }
    return ret;
}

zinline void zlist_destroy(zlist_t *list){
    if(!list){
        return;
    }
    zatm_free(list->size);
    zalloc_destroy(list->alloc);
    free(list);
}

zinline zerr_t zlist_alloc_node(zlist_t *list, znod_t **nod){
    return zalloc_pop(list->alloc, (zptr_t*)nod);
}

zinline zerr_t zlist_recycle_node(zlist_t *list, znod_t *nod){
    return zalloc_push(list->alloc, (zptr_t*)&nod);
}

zinline zerr_t zlist_push(zlist_t *list, znod_t *nod){
    if(1 == zatm_inc(list->size)){
        list->head = list->tail = nod;
        nod->prev = nod->next = NULL;
    }else{
        nod->next = NULL;
        nod->prev = list->tail;

        list->tail->next = nod;
        list->tail = nod;
    }
    return ZEOK;
}

zinline zerr_t zlist_pop(zlist_t *list, znod_t **nod){
    zerr_t ret = ZENOT_EXIST;
    if(zatm_dec(list->size) >= 0){
        *nod = list->head;
        list->head = list->head->next;
        if(list->head){
            list->head->prev = NULL;
        }
        ret = ZEOK;
    }else{
        zatm_inc(list->size);
    }
    return ret;
}

zinline zerr_t zlist_push_front(zlist_t *list, znod_t *nod){
    if(1 == zatm_inc(list->size)){
        list->head = list->tail = nod;
        nod->prev = nod->next = NULL;
    }else{
        nod->next = list->head;
        nod->prev = NULL;

        list->head->prev = nod;
        list->head = nod;
    }
    return ZEOK;
}

zinline zerr_t zlist_pop_back(zlist_t *list, znod_t **nod){
    zerr_t ret = ZENOT_EXIST;
    if(zatm_dec(list->size) >= 0){
        *nod = list->tail;
        list->tail = list->tail->prev;
        if(list->tail){
            list->tail->next = NULL;
        }
        ret = ZEOK;
    }else{
        zatm_inc(list->size);
    }
    return ret;
}

zinline zerr_t zlist_foreach(zlist_t *list, zoperate op,
                             zvalue_t hint){
    znod_t *nod = list->head;
    while(nod){
        op((zvalue_t)nod, NULL, hint);
        nod = nod->next;
    }
    return ZEOK;
}

zinline int32_t zlist_size(zlist_t *list){
    return *list->size;
}

zinline zerr_t zlist_back(zlist_t *list, znod_t **nod){
    if(*list->size){
        *nod = list->tail;
        return ZEOK;
    }
    return ZENOT_EXIST;
}

zinline zerr_t zlist_front(zlist_t *list, znod_t **nod){
    if(*list->size){
        *nod = list->head;
        return ZEOK;
    }
    return ZENOT_EXIST;
}

/*
 * mulit thread safe push/pop
 */
zinline zerr_t zlist_lock_push(zlist_t *list, znod_t *nod,
                               zspinlock_t *spin){
    zerr_t ret = ZEOK;

    zspin_lock(spin);
    ret = zlist_push(list, nod);
    zspin_unlock(spin);

    return ret;
}

zinline zerr_t zlist_lock_pop(zlist_t *list, znod_t **pnod,
                              zspinlock_t *spin){
    zerr_t ret = ZEOK;

    zspin_lock(spin);
    ret = zlist_pop(list, pnod);
    zspin_unlock(spin);

    return ret;
}

zinline zerr_t zlist_lock_push_front(zlist_t *list, znod_t *nod,
                                     zspinlock_t *spin){
    zerr_t ret = ZEOK;

    zspin_lock(spin);
    ret = zlist_push_front(list, nod);
    zspin_unlock(spin);

    return ret;
}

zinline zerr_t zlist_lock_pop_back(zlist_t *list, znod_t **pnod,
                                   zspinlock_t *spin){
    zerr_t ret = ZEOK;

    zspin_lock(spin);
    ret = zlist_pop_back(list, pnod);
    zspin_unlock(spin);

    return ret;
}

ZC_END

#endif
