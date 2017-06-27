#ifndef _ZIT_DLFCN_H_
#define _ZIT_DLFCN_H_

#include "platform.h"
#include <zit/base/type.h>

#ifdef ZSYS_POSIX
#include <dlfcn.h>
//#include <link.h>

#define zdl_t void*
// need link -ldl
#else

#include <Windows.h>
#define zdl_t HINSTANCE
#define RTLD_LAZY 1
#endif

ZC_BEGIN

#if !ZUSE_INLINE
ZAPI zdl_t zdl_open(const char *filename, int flags); // flag=RTLD_LAZY
ZAPI int zdl_close(zdl_t dl);
ZAPI zvalue_t zdl_sym(zdl_t dl, const char *symbol);
ZAPI char * zdl_error(void); // NULL: no error

//ZAPI int zdl_info(zdl_t dl, int request, zvalue_t info);
//ZAPI int zdl_addr(zdl_t dl, zvalue_t info);

#else

#include <zit/base/error.h>

inline zdl_t zdl_open(const char *filename, int flags){
#ifdef ZSYS_POSIX
    zdl_t dl =  dlopen(filename, flags);
    return dlerror() ? NULL : dl;
#else
    return LoadLibrary(filename);
#endif
}

inline int zdl_close(zdl_t dl){
#ifdef ZSYS_POSIX
    return dlclose(dl) ? ZFUN_FAIL : ZOK;
#else
    return FreeLibrary(hInst) ? ZOK : ZFUN_FAIL;;
#endif
}

inline zvalue_t zdl_sym(zdl_t dl, const char *symbol){
 #ifdef ZSYS_POSIX
     zvalue_t sym =  dlsym(dl, symbol);
     return dlerror() ? NULL : sym;
#else
    return (zvalue_t)GetProcAddress(dl, symbol);
#endif
}


inline char * zdl_error(void){ // NULL: no error
#ifdef ZSYS_POSIX
    return dlerror();
#else
    return NULL;
#endif
}

#endif // ZUSE_INLINE

ZC_END

#endif
