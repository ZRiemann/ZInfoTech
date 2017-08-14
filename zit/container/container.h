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

ZC_BEGIN

#define ZCONT_TYPE_QUEUE 0
#define ZCONT_TYPE_LIST 1
#define ZCONT_TYPE_QUE_MT 2
#define ZCONT_TYPE_LIST_MT 3

ZAPI zerr_t zcontainer_create(zcontainer_t *cont, int type, void *hint);
ZAPI zerr_t zcontainer_destroy(zcontainer_t cont);
ZAPI zerr_t zcontainer_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zcontainer_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zcontainer_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zcontainer_popback(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zcontainer_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zcontainer_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zcontainer_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zcontainer_size(zcontainer_t cont);
ZAPI zerr_t zcontainer_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zcontainer_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zcontainer_swap(zcontainer_t cont1, zcontainer_t cont2);

ZC_END

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
