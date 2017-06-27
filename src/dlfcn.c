#include "export.h"
#include <zit/base/platform.h>

#if !ZUSE_INLINE

#include <zit/base/dlfcn.h>
#include <zit/base/trace.h>

#ifdef ZSYS_POSIX
//#include <dlfcn.h>
//#include <link.h>
#else // ZSYS_WINDOWS
#include <Windows.h>
#endif

zdl_t zdl_open(const char *filename, int flags){
    zdl_t dl;
#ifdef ZSYS_POSIX
    dl = dlopen(filename, flags);
    if(dl){
        dlerror(); // Clear any existing error
    }
#else
    dl = LoadLibrary(filename);
#endif
    ZDBG("zdl_open(name:%s, flags:%d):%p", filename, flags, dl);
    return dl;
}
int zdl_close(zdl_t dl){
    ZDBG("zdl_close(%p)",dl);
#ifdef ZSYS_POSIX
    return dlclose(dl) ? ZFUN_FAIL : ZOK;
#else
    return FreeLibrary(hInst) ? ZOK : ZFUN_FAIL;;
#endif

}
zvalue_t zdl_sym(zdl_t dl, const char *symbol){
    zvalue_t sym;
    char *error;
#ifdef ZSYS_POSIX
    sym = dlsym(dl, symbol);
    if(NULL != (error = dlerror())){
        sym = NULL;
        ZERR("%s", error);
    }
#else
    sym = (zvalue_t)GetProcAddress(dl, symbol);
#endif
    ZDBG("zdl_sym(dl:%p, symbol:%s):%p", dl, symbol, sym);
    return sym;
}

char * zdl_error(void){ // NULL: no error
    char *error;
#ifdef ZSYS_POSIX
    error = dlerror();
    if(error){
        ZDBG("zdl_drror(): %s", error);
    }
#else
    error = NULL;
#endif
    return error;
}

//ZAPI int zdl_info(zdl_t dl, int request, zvalue_t info);
//ZAPI int zdl_addr(zdl_t dl, zvalue_t info);
#endif