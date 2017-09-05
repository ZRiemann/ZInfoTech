#ifndef _ZCONT_LIST_MT_H_
#define _ZCONT_LIST_MT_H_
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
 * @file zit/container/queue_mt.h
 * @brief An effeicient list for n thread push and n thread pop. based list
 * @details
 *  - thread thread for multithread push()/pop()
 *  - single thread access list, suggest use list  it's no lock;
 */
#include <zit/base/type.h>

ZC_BEGIN

ZAPI zerr_t zlist_mt_create(zcontainer_t *cont, zvalue_t hint);
ZAPI zerr_t zlist_mt_destroy(zcontainer_t cont);
ZAPI zerr_t zlist_mt_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zlist_mt_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zlist_mt_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zlist_mt_popback(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_mt_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zlist_mt_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zlist_mt_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zlist_mt_size(zcontainer_t cont);
ZAPI zerr_t zlist_mt_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_mt_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zlist_mt_swap(zcontainer_t *cont1, zcontainer_t *cont2);

ZC_END

#endif
