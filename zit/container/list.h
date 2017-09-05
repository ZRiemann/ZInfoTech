#ifndef _ZIT_LIST_H_
#define _ZIT_LIST_H_
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

ZC_BEGIN

ZAPI zerr_t zlist_create(zcontainer_t *cont, zvalue_t hint);
ZAPI zerr_t zlist_destroy(zcontainer_t cont);
ZAPI zerr_t zlist_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zlist_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zlist_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zlist_popback(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zlist_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zlist_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zlist_size(zcontainer_t cont);
ZAPI zerr_t zlist_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_swap(zcontainer_t *cont1, zcontainer_t *cont2);

ZC_END

#endif
