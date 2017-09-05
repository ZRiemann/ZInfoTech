#ifndef _ZUTILITY_MODULE_H_
#define _ZUTILITY_MODULE_H_
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
#include <zit/base/type.h>

ZC_BEGIN

ZAPI int zmodule_name(char* path, char* name);

ZC_END
/**@fn int zmodule_name(char* path, char* name)
   @brief get the executable module path and name
   @param char* path [out] buffer for path[256]
   @param char* name [out] buffer for name[64]
   @return ZEOK|ZENOT_EXIST|[errno|lasterror]
   @note if path set NULL, ignore path, so seem to name.
 */

#endif
