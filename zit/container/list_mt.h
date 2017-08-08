#ifndef _ZCONT_LIST_MT_H_
#define _ZCONT_LIST_MT_H_
/**
 * @file zit/container/queue_mt.h
 * @brief An effeicient list for n thread push and n thread pop. based list
 * @details
 *  - thread thread for multithread push()/pop()
 *  - single thread access list, suggest use list  it's no lock;
 */
#include "list.h"
#include <zit/thread/spin.h>

typedef struct zlist_mt_s{
    zcontainer_t cont;
    zspinlock_t front_spin;
    zspinlock_t back_spin;
}zlist_mt_t;

zerr_t zlist_mt_create(zcontainer_t *cont);
zerr_t zlist_mt_destroy(zcontainer_t cont);
zerr_t zlist_mt_push(zcontainer_t cont, zvalue_t in); // push back
zerr_t zlist_mt_pop(zcontainer_t cont, zvalue_t *out); // pop front
zerr_t zlist_mt_pushfront(zcontainer_t cont, zvalue_t in);
zerr_t zlist_mt_popback(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_mt_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
zerr_t zlist_mt_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
zerr_t zlist_mt_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
size_t zlist_mt_size(zcontainer_t cont);
zerr_t zlist_mt_back(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_mt_front(zcontainer_t cont, zvalue_t *out);
zerr_t zlist_mt_swap(zcontainer_t *cont1, zcontainer_t *cont2);

zinline zerr_t zlist_mt_create(zcontainer_t *cont){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)zmem_align64(sizeof(zlist_mt_t));
    do{
        if(!list){
            ret = ZEMEM_INSUFFICIENT;
            break;
        }
        if(ZOK != (ret = zlist_create(&list->cont)))break;
        zspin_init(&list->front_spin);
        zspin_init(&list->back_spin);
        *cont = list;
    }while(0);
    return ret;
}

zinline zerr_t zlist_mt_destroy(zcontainer_t cont){
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

zinline zerr_t zlist_mt_push(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_push(list->cont, in);
    zspin_unlock(&list->back_spin);
    return ret;
}

zinline zerr_t zlist_mt_pop(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_pop(list->cont, out);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline zerr_t zlist_mt_pushfront(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_pushfront(list->cont, in);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline zerr_t zlist_mt_popback(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_popback(list->cont, out);
    zspin_unlock(&list->back_spin);
    return ret;
}

zinline zerr_t zlist_mt_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_insert(list->cont, in, compare, condition);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline zerr_t zlist_mt_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_erase(list->cont, hint, compare, condition);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline zerr_t zlist_mt_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    zspin_lock(&list->back_spin);
    ret = zlist_foreach(list->cont, op, hint);
    zspin_unlock(&list->back_spin);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline size_t zlist_mt_size(zcontainer_t cont){
    zlist_mt_t *list = (zlist_mt_t*)cont;
    return zlist_size(list->cont);
}

zinline zerr_t zlist_mt_back(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->back_spin);
    ret = zlist_back(list->cont, out);
    zspin_unlock(&list->back_spin);
    return ret;
}

zinline zerr_t zlist_mt_front(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zlist_mt_t *list = (zlist_mt_t*)cont;
    zspin_lock(&list->front_spin);
    ret = zlist_front(list->cont, out);
    zspin_unlock(&list->front_spin);
    return ret;
}

zinline zerr_t zlist_mt_swap(zcontainer_t *cont1, zcontainer_t *cont2){
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

#endif
