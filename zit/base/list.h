#ifndef _ZIT_LIST_H_
#define _ZIT_LIST_H_
/**@file zit/base/list.h
 * @biref list implement
 * @note
 * @email 25452483@qq.com
 * @history
 * 2016-9-12 ZRiemann found
 */
#include <zit/base/type.h>
ZC_BEGIN

ZAPI int zlist_create(zcontainer_t *cont);
ZAPI int zlist_destroy(zcontainer_t cont, zoperate release);
ZAPI int zlist_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI int zlist_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI int zlist_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI int zlist_popback(zcontainer_t cont, zvalue_t *out);
ZAPI int zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare);
ZAPI int zlist_erase(zcontainer_t cont, zvalue_t in, zoperate compare);
ZAPI int zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI zsize_t zlist_size(zcontainer_t cont);
ZC_END
#endif
