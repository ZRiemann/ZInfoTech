#ifndef _ZIT_DLFCN_H_
#define _ZIT_DLFCN_H_
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

#ifdef ZSYS_POSIX
#include <dlfcn.h>
//#include <link.h>
typedef void* zdl_t;
// need link -ldl
#else
#include <Windows.h>
typedef HINSTANCE zdl_t;
#endif

ZC_BEGIN

ZAPI zdl_t zdl_open(const char *filename);
ZAPI int zdl_close(zdl_t dl);
ZAPI zvalue_t zdl_sym(zdl_t dl, const char *symbol);
ZAPI char * zdl_error(void); // NULL: no error

ZC_END

#endif
