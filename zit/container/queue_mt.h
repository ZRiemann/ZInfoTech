#ifndef _ZCONT_QUE_MT_H_
#define _ZCONT_QUE_MT_H_
/**
 * @file zit/container/queue_mt.h
 * @brief An effeicient queue for n thread push and n thread pop. based queue
 * @details
 *  - thread thread for multithread push()/pop()
 *  - single thread access queue, suggest use queue_1r1w it's no lock;
 */
#include <zit/container/queue.h>
#include <zit/thread/spin.h>

typedef struct zqueue_nrnw_s{
    zcontainer_t cont;
    zspinlock_t front_spin;
    zspinlock_t back_spin;
}zquen_t;

zerr_t zquen_create(zcontainer_t *cont);
zerr_t zquen_destroy(zcontainer_t cont);
zerr_t zquen_push(zcontainer_t cont, zvalue_t in); // push back
zerr_t zquen_pop(zcontainer_t cont, zvalue_t *out); // pop front
zerr_t zquen_pushfront(zcontainer_t cont, zvalue_t in);
zerr_t zquen_popback(zcontainer_t cont, zvalue_t *out);
zerr_t zquen_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
zerr_t zquen_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
zerr_t zquen_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
zsize_t zquen_size(zcontainer_t cont);
zerr_t zquen_back(zcontainer_t cont, zvalue_t *out);
zerr_t zquen_front(zcontainer_t cont, zvalue_t *out);
zerr_t zquen_swap(zcontainer_t *cont1, zcontainer_t *cont2);

inline zerr_t zquen_create(zcontainer_t *cont){
    zerr_t ret;
    zquen_t *que = (zquen_t*)zmem_align64(sizeof(zquen_t));
    do{
        if(!que){
            ret = ZEMEM_INSUFFICIENT;
            break;
        }
        if(ZOK != (ret = zque1_create(&que->cont)))break;
        zspin_init(&que->front_spin);
        zspin_init(&que->back_spin);
        *cont = que;
    }while(0);
    return ret;
}

inline zerr_t zquen_destroy(zcontainer_t cont){
    zquen_t *que;
    if(!cont){
        return ZEOK;
    }
    que = (zquen_t*)cont;
    zque1_destroy(que->cont);
    zspin_fini(&que->front_spin);
    zspin_fini(&que->back_spin);
    free(cont);
    return ZEOK;
}

inline zerr_t zquen_push(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_push(que->cont, in);
    zspin_unlock(&que->back_spin);
    return ret;
}

inline zerr_t zquen_pop(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_pop(que->cont, out);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zerr_t zquen_pushfront(zcontainer_t cont, zvalue_t in){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_pushfront(que->cont, in);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zerr_t zquen_popback(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_popback(que->cont, out);
    zspin_unlock(&que->back_spin);
    return ret;
}

inline zerr_t zquen_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_insert(que->cont, in, compare, condition);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zerr_t zquen_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_erase(que->cont, hint, compare, condition);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zerr_t zquen_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    zspin_lock(&que->back_spin);
    ret = zque1_foreach(que->cont, op, hint);
    zspin_unlock(&que->back_spin);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zsize_t zquen_size(zcontainer_t cont){
    zquen_t *que = (zquen_t*)cont;
    return zque1_size(que->cont);
}

inline zerr_t zquen_back(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->back_spin);
    ret = zque1_back(que->cont, out);
    zspin_unlock(&que->back_spin);
    return ret;
}

inline zerr_t zquen_front(zcontainer_t cont, zvalue_t *out){
    zerr_t ret;
    zquen_t *que = (zquen_t*)cont;
    zspin_lock(&que->front_spin);
    ret = zque1_front(que->cont, out);
    zspin_unlock(&que->front_spin);
    return ret;
}

inline zerr_t zquen_swap(zcontainer_t *cont1, zcontainer_t *cont2){
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

#endif
