#ifndef _ZCONT_QUE_MT_H_
#define _ZCONT_QUE_MT_H_
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
 * @brief An effeicient queue for n thread push and n thread pop. based queue
 * @details
 *  - thread thread for multithread push()/pop()
 *  - single thread access queue, suggest use queue_1r1w it's no lock;
 */
#include <zit/base/type.h>

ZC_BEGIN

ZAPI zerr_t zquen_create(zcontainer_t *cont, void *hint);
ZAPI zerr_t zquen_destroy(zcontainer_t cont);
ZAPI zerr_t zquen_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zquen_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zquen_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zquen_popback(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zquen_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zquen_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zquen_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zquen_size(zcontainer_t cont);
ZAPI zerr_t zquen_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zquen_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zquen_swap(zcontainer_t *cont1, zcontainer_t *cont2);

ZC_END

#endif
