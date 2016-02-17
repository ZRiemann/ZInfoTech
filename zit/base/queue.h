#ifndef _ZBASE_QUEUE_H_
#define _ZBASE_QUEUE_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>

ZEXP int zqueue_create(zcontainer_t* cont);
ZEXP int zqueue_destroy(zcontainer_t* cont);
ZEXP int zqueue_push(zcontainer_t cont, int back, zvalue_t data, zcompare cmp);
ZEXP int zqueue_pop(zcontainer_t cont, int back, zvalue_t* data, zcompare cmp);
// extern queue api
ZEXP int zqueue_pushback(zcontainer_t cont, zvalue_t value);
ZEXP int zqueue_pushfront(zcontainer_t cont, zvalue_t value);
ZEXP int zqueue_popback(zcontainer_t cont, zvalue_t* value);
ZEXP int zqueue_popfornt(zcontainer_t cont, zvalue_t* value);
ZEXP int zqueue_foreach(zcontainer_t cont, ztsk_t* tsk);
/**@fn int zqueue_push(zcontainer_t cont, int back, void* data, zcompare cmp)
   @brief push <data> into <cont> back or front or insert positioned by <cmp>
   @param zcontainer_t cont [in] queue handle
   @param int back [in] ZBACK-push front ZFRONT-push back
   @param void* data [in] user data
   @param zcompare cmp [in] compare condition ZGREAT/ZEQUAL/ZLITTLE
   @return ZEOK
   @note zcompare set NULL, just push front or back
 */
/**@fn int zqueue_pop(zcontainer_t cont, int back, void* data, zcompare cmp)
   @brief pop <data> from <cont> front or back or erase data positioned by <cmp>
   @param zcontainer_t cont [int] queue handle
   @param int back [in] ZBACK-push back, ZFRONT-push front
   @param void** data [out|in] pop data or erase data by cmp
   @param zcompare cmp [in] condition
   @return ZEOK/ZENOT_EXIST
   @note zcompare set NULL, just pop data.
 */

#endif
