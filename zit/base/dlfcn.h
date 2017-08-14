#ifndef _ZIT_DLFCN_H_
#define _ZIT_DLFCN_H_

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
