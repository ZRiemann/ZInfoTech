#ifndef _ZBASE_CONTAINER_H_
#define _ZBASE_CONTAINER_H_
/**
 * @file zit/container/container.h
 * @brief a container interface, support multi type container.
 * @details
 *  Try $ make/release/zit_test container
 *  ZCONT_TYPE_QUEUE 0.452631
 *  ZCONT_TYPE_LIST  1.471623
 *  ZCONT_TYPE_QUE_MT 1.069935
 *  ZCONT_TYPE_LIST_MT 1.528248
 */
#include <zit/base/type.h>

//ZC_BEGIN

#define ZCONT_TYPE_QUEUE 0
#define ZCONT_TYPE_LIST 1
#define ZCONT_TYPE_QUE_MT 2
#define ZCONT_TYPE_LIST_MT 3

zerr_t zcontainer_create(zcontainer_t *cont, int type);
zerr_t zcontainer_destroy(zcontainer_t cont);
zerr_t zcontainer_push(zcontainer_t cont, zvalue_t in); // push back
zerr_t zcontainer_pop(zcontainer_t cont, zvalue_t *out); // pop front
zerr_t zcontainer_pushfront(zcontainer_t cont, zvalue_t in);
zerr_t zcontainer_popback(zcontainer_t cont, zvalue_t *out);
zerr_t zcontainer_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
zerr_t zcontainer_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
zerr_t zcontainer_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
size_t zcontainer_size(zcontainer_t cont);
zerr_t zcontainer_back(zcontainer_t cont, zvalue_t *out);
zerr_t zcontainer_front(zcontainer_t cont, zvalue_t *out);
zerr_t zcontainer_swap(zcontainer_t cont1, zcontainer_t cont2);

#include <zit/container/list.h>
#include <zit/container/queue.h>
#include <zit/container/list_mt.h>
#include <zit/container/queue_mt.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

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

zinline zerr_t zcontainer_create(zcontainer_t *cont, int type){
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
        ret = zlist_create(&cnt->cont);
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
        ret = zque1_create(&cnt->cont);
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
        ret = zquen_create(&cnt->cont);
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
        ret = zlist_mt_create(&cnt->cont);
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
zinline zerr_t zcontainer_destroy(zcontainer_t cont){
    zcont_t *cnt = (zcont_t*)cont;
    if(!cont){
        return ZEOK;
    }
    cnt->destroy(cnt->cont);
	zmem_align_free(cnt);
	return ZEOK;
}

zinline zerr_t zcontainer_push(zcontainer_t cont, zvalue_t in){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->push(cnt->cont, in));
}
zinline zerr_t zcontainer_pop(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->pop(cnt->cont, out));
}
zinline zerr_t zcontainer_pushfront(zcontainer_t cont, zvalue_t in){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->pushfront(cnt->cont, in));
}
zinline zerr_t zcontainer_popback(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->popback(cnt->cont, out));
}
zinline zerr_t zcontainer_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->insert(cnt->cont, in, compare, condition));
}
zinline zerr_t zcontainer_erase(zcontainer_t cont, zvalue_t in, zoperate compare, int condition){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->erase(cnt->cont, in, compare, condition));
}
zinline zerr_t zcontainer_foreach(zcontainer_t cont, zoperate op, zvalue_t hint){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->foreach(cnt->cont, op, hint));
}

zinline size_t zcontainer_size(zcontainer_t cont){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->size(cnt->cont));
}

zinline zerr_t zcontainer_back(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->back(cnt->cont, out));
}
zinline zerr_t zcontainer_front(zcontainer_t cont, zvalue_t *out){
    zcont_t *cnt = (zcont_t*)cont;
    //ZASSERT(!cnt);
    return(cnt->front(cnt->cont, out));
}

//#endif // ZUSE_INLINE
//ZC_END

/**@file container.h
   @brief general container
   @note 
   auther   date       description
   ZRiemann 2016-02-18 found
*/
/**@fn zerr_t zcontainer_push(zcontainer_t cont, int back, void* data, zcompare cmp)
   @brief push <data> into <cont> back or front or insert positioned by <cmp>
   @param zcontainer_t cont [in] queue handle
   @param int back [in] ZBACK-push front ZFRONT-push back
   @param void* data [in] user data
   @param zcompare cmp [in] compare condition ZGREAT/ZEQUAL/ZLITTLE
   @return ZEOK
   @note zcompare set NULL, just push front or back
*/
/**@fn int zcontainer_pop(zcontainer_t cont, int back, void* data, zcompare cmp)
   @brief pop <data> from <cont> front or back or erase data positioned by <cmp>
   @param zcontainer_t cont [int] queue handle
   @param int back [in] ZBACK-push back, ZFRONT-push front
   @param void** data [out|in] pop data or erase data by cmp
   @param zcompare cmp [in] condition
   @return ZEOK/ZENOT_EXIST
   @note zcompare set NULL, just pop data.
*/

#endif
