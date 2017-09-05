#ifndef _ZUTILITY_TRACE_RING_H_
#define _ZUTILITY_TRACE_RING_H_
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

#include <zit/base/platform.h>
#include <zit/base/trace.h>

ZC_BEGIN

ZAPI int ztracering_init(ztrace trace, void* user);
ZAPI int ztracering_uninit();
ZC_END

/**@file zit/utility/tracering.h
   @brief trace in back ground by jet and ringbuf
*/
#endif
