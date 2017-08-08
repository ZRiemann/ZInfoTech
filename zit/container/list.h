#ifndef _ZIT_LIST_H_
#define _ZIT_LIST_H_
/**
 * @file zit/base/list.h
 * @biref list implement
 * @author ZRiemann
 * @email 25452483@qq.com
 * @date 2016-9-12 ZRiemann found
 * @details
 *  - Support one thread push and another thread pop parallel;
 *  - Support node memory recycle.
 *  - zlist_insert() , zlist_foreach()  zlist_erase()
 *    not thread safe, suggest single thread access or use list_mt
 *  - It's about 7000000 by 1 push thread and 1 pop thread per second, on (vm)ubuntu17
 *    Try ($ make/release/zit_test queue_1r1w) interval:0.499194 10 million
 *    Contrast zpin+std::queue with queue_1r1w push/pop 10 million;
 *    Try ($ make/release/zpp_test queue_1r1w) interval:1.036969 10 million
 *    Try ($ make/release/zit_test list_1r1w) interval:1.524639 10 million
 *    Try ($ make/release/zpp_test list_1r1w) interval:1.655607 10 million
 */

#include <zit/base/type.h>
//ZC_BEGIN

zerr_t zlist_create(zcontainer_t *cont);
zerr_t zlist_destroy(zcontainer_t cont);
zerr_t zlist_push(zcontainer_t cont, zvalue_t in); // push back
zerr_t zlist_pop(zcontainer_t cont, zvalue_t *out); // pop front
zerr_t zlist_pushfront(zcontainer_t cont, zvalue_t in);
zerr_t zlist_popback(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
zerr_t zlist_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
zerr_t zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
size_t zlist_size(zcontainer_t cont);
zerr_t zlist_back(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_front(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_swap(zcontainer_t *cont1, zcontainer_t *cont2);
/*
 * implements
 */
#include <malloc.h>
#include <string.h>
#include <zit/thread/spin.h>

/**
 * @brief zlist structure
 */
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

zinline zerr_t zlist_popnode(zlist_t *list,  znod_t **nod){
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
zinline zerr_t zlist_pushnode(zlist_t *list,  znod_t *nod){
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
zinline zerr_t zlist_freenode(znod_t *back){
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
zinline zerr_t zlist_create(zcontainer_t *cont){
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

zinline zerr_t zlist_destroy(zcontainer_t cont){
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

zinline zerr_t zlist_push(zcontainer_t cont, zvalue_t in){
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
zinline zerr_t zlist_pop(zcontainer_t cont, zvalue_t *out){
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

zinline zerr_t zlist_pushfront(zcontainer_t cont, zvalue_t in){
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

zinline zerr_t zlist_popback(zcontainer_t cont, zvalue_t *out){
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

zinline zerr_t zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
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

zinline zerr_t zlist_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
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

zinline zerr_t zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zlist_t *list = (zlist_t*)cont;
    znod_t *nod = list->head;

    while(nod->next && nod->next->next){
        op(nod->next->value, NULL, hint);
        nod = nod->next;
    }
    return ZEOK;
}

zinline size_t zlist_size(zcontainer_t cont){
    return *((zlist_t*)cont)->size;
}
zinline zerr_t zlist_back(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    if(*list->size){
        *out = list->tail->prev->value;
        return ZEOK;
    }
    return ZENOT_EXIST;
}
zinline zerr_t zlist_front(zcontainer_t cont, zvalue_t *out){
    zlist_t *list = (zlist_t*)cont;
    if(*list->size){
        *out = list->head->next->value;
        return ZEOK;
    }
    return ZENOT_EXIST;
}

zinline zerr_t zlist_swap(zcontainer_t *cont1, zcontainer_t *cont2){
    *cont2 = zatm_xchg_ptr(cont1, *cont2);
    return ZEOK;
}

//ZC_END

#endif
