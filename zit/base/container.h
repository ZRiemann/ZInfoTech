#ifndef _ZBASE_CONTAINER_H_
#define _ZBASE_CONTAINER_H_

#include <zit/base/type.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZCONTAINER_LIST 0
#define ZCONTAINER_QUEUE 1
#define ZCONTAINER_DEQUEUE 2
 
ZEXP int zcontainer_create(zcontainer_t* cont, int type);
ZEXP int zcontainer_destroy(zcontainer_t* cont);

ZEXP int zcontainer_push(zcontainer_t cont, int back, zvalue_t data, zcompare cmp);
ZEXP int zcontainer_pop(zcontainer_t cont, int back, zvalue_t* data, zcompare cmp);
// extern container api
ZEXP int zcontainer_pushback(zcontainer_t cont, zvalue_t value);
ZEXP int zcontainer_pushfront(zcontainer_t cont, zvalue_t value);
ZEXP int zcontainer_popback(zcontainer_t cont, zvalue_t* value);
ZEXP int zcontainer_popfornt(zcontainer_t cont, zvalue_t* value);
ZEXP int zcontainer_foreach(zcontainer_t cont, ztsk_t* tsk);

#ifdef __cplusplus
}
#endif

/**@file container.h
   @brief general container
   @note 
   auther   date       description
   ZRiemann 2016-02-18 found
 */
/**@fn int zcontainer_push(zcontainer_t cont, int back, void* data, zcompare cmp)
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
