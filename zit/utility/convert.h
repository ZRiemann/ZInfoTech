#ifndef _ZUTILITY_CONVERT_H_
#define _ZUTILITY_CONVERT_H_
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

ZC_BEGIN

ZAPI int zconv_u2g(int isu2g, char *in, int inlen, char *out, int *outlen);

ZC_END

#endif
