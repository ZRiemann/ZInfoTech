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
#include "export.h"
#include <zit/base/dlfcn.h>
#include <zit/base/filesys.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
/*
 * implement
 */
zdl_t zdl_open(const char *filename){
#ifdef ZSYS_POSIX
    zdl_t dl =  dlopen(filename, RTLD_LAZY);
    //return dlerror() ? NULL : dl;
    if(!dl){
        ZDBG("%s", dlerror());
    }
    return dl;
#else
    return LoadLibrary(filename);
#endif
}

int zdl_close(zdl_t dl){
#ifdef ZSYS_POSIX
    return dlclose(dl) ? ZFUN_FAIL : ZOK;
#else
    return FreeLibrary(dl) ? ZOK : ZFUN_FAIL;;
#endif
}

zvalue_t zdl_sym(zdl_t dl, const char *symbol){
    if(!dl){
        return NULL;
    }
 #ifdef ZSYS_POSIX
     zvalue_t sym =  dlsym(dl, symbol);
     return dlerror() ? NULL : sym;
#else
    return (zvalue_t)GetProcAddress(dl, symbol);
#endif
}


char * zdl_error(void){ // NULL: no error
#ifdef ZSYS_POSIX
    return dlerror();
#else
    return NULL;
#endif
}

/*============================================================================*/
/*
 * implement plugin
 */
#include <zit/framework/plugin.h>
zerr_t zplg_dummy(ZOP_ARG){return 0;}
zerr_t zplg_notsupport(ZOP_ARG){return -1;}
zerr_t zplg_scan(char path[512], const char *prefix, cbzftw ftw){
    return zftw_nr(path , ftw, (zvalue_t)prefix);
}

zerr_t zplg_open(zplg_itf_t *itf, const char *filename){
    zvalue_t sym;
    /* assert(itf,filename) */
    itf->dl = zdl_open(filename);
    /* assert(dl)*/
    itf->init = (sym = zdl_sym(itf->dl, "init")) ? (zoperate)sym : zplg_dummy;
    itf->fini = (sym = zdl_sym(itf->dl, "fini")) ? (zoperate)sym : zplg_dummy;
    itf->run = (sym = zdl_sym(itf->dl, "run")) ? (zoperate)sym : zplg_dummy;
    itf->stop = (sym = zdl_sym(itf->dl,"stop")) ? (zoperate)sym : zplg_dummy;
    itf->operate = (sym = zdl_sym(itf->dl,"operate")) ? (zoperate)sym : zplg_dummy;
    itf->ability = (sym = zdl_sym(itf->dl, "ability")) ? (zoperate)sym :zplg_notsupport;

    return itf->dl ? ZOK : ZFAIL;
}

zerr_t zplg_close(zplg_itf_t *itf){
    return zdl_close(itf->dl);
}

zerr_t zplg_init(zplg_t *plg, zoperate cb, zvalue_t cb_hint, zvalue_t hint){
    zvalue_t values[2];
    values[0] = (zvalue_t)cb;
    values[1] = cb_hint;
    return plg->itf_init((zvalue_t)values, &plg->handle, hint);
}

zerr_t zplg_fini(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_fini(plg->handle, out, hint);
}

zerr_t zplg_run(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_run(plg->handle, out, hint);
}

zerr_t zplg_stop(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_stop(plg->handle, out, hint);
}

zerr_t zplg_operate(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_operate(plg->handle, out, hint);
}

zerr_t zplg_ability(zplg_itf_t *itf, zvalue_t *out, zvalue_t hint){
    return itf->ability((zvalue_t)itf, out, hint);
}
