#ifndef _ZIT_FRAMEWORK_PLUGIN_H_
#define _ZIT_FRAMEWORK_PLUGIN_H_
/**
 * @file zit/framesork/plugin.h
 * @brief plugin module definition
 * @note
 *  C program = tructure + algorithm;
 *  C code = data + interface;
 *
 * @email 25452483@qq.com
 * @history
 *  2017-07-05 ZRiemann found
 */
#include <zit/base/type.h>
#include <zit/base/dlfcn.h>
#include <zit/base/filesys.h>

int zplg_dummy(ZOP_ARG);
zinline int zplg_dummy(ZOP_ARG){return 0;}
int zplg_notsupport(ZOP_ARG);
zinline int zplg_notsupport(ZOP_ARG){return -1;}

typedef struct zplugin_interface_s{
    zoperate init;
    zoperate fini;
    zoperate run;
    zoperate stop;
    zoperate operate;
    zoperate ability; // get plugin ability;

    // private member, user never access
    zdl_t dl; // plugin handle

    zotype_t type; // plugin type
    zotype_t version; // 0x00<major><minor><revision>
}zplg_itf_t;

typedef struct zplugin_s{
    zplg_itf_t *itf;

    // private member, user never access
    zvalue_t handle;
}zplg_t;

#define itf_init itf->init
#define itf_fini itf->fini
#define itf_run itf->run
#define itf_stop itf->stop
#define itf_operate itf->operate
#define itf_ability itf->ability

#include <zit/base/filesys.h>

int zplg_scan(char path[512], const char *prefix, cbzftw ftw);
int zplg_open(zplg_itf_t *itf, const char *filename);
int zplg_ability(zplg_itf_t *itf, zvalue_t *out, zvalue_t hint);
int zplg_close(zplg_itf_t *itf);
// plugin operates
int zplg_init(zplg_t *plg, zoperate cb, zvalue_t cb_hint, zvalue_t hint);
int zplg_fini(zplg_t *plg, zvalue_t *out, zvalue_t hint);
int zplg_run(zplg_t *plg, zvalue_t *out, zvalue_t hint);
int zplg_stop(zplg_t *plg, zvalue_t *out, zvalue_t hint);
int zplg_operate(zplg_t *plg, zvalue_t *out, zvalue_t hint);


/*
 * implement plugin
 */
zinline int zplg_scan(char path[512], const char *prefix, cbzftw ftw){
    return zftw_nr(path , ftw, (zvalue_t)prefix);
}
#if 0
int zplg_ftw(const char *pathname, zfstat_t *stat, int ftw_flag, zvalue_t hint){
    const char *prefix = (const char*)hint;

    if((ftw_flag == FTW_F) && strstr(prefix)                            \
        && ((strstr(".so", pathname) - pathname) == (strlen(pahtname)-3))){
        zplg_itf_t itf;
        zplg_open(itf, pathname);
        //itf.ability(...);
    }

  return 0;
}
#endif

zinline int zplg_open(zplg_itf_t *itf, const char *filename){
    zvalue_t sym;
    /* assert(itf,filename) */
    itf->dl = zdl_open(filename, RTLD_LAZY);
    /* assert(dl)*/
    itf->init = (sym = zdl_sym(itf->dl, "init")) ? (zoperate)sym : zplg_dummy;
    itf->fini = (sym = zdl_sym(itf->dl, "fini")) ? (zoperate)sym : zplg_dummy;
    itf->run = (sym = zdl_sym(itf->dl, "run")) ? (zoperate)sym : zplg_dummy;
    itf->stop = (sym = zdl_sym(itf->dl,"stop")) ? (zoperate)sym : zplg_dummy;
    itf->operate = (sym = zdl_sym(itf->dl,"operate")) ? (zoperate)sym : zplg_dummy;
    itf->ability = (sym = zdl_sym(itf->dl, "ability")) ? (zoperate)sym :zplg_notsupport;

    return itf->dl ? ZOK : ZFAIL;
}

zinline int zplg_close(zplg_itf_t *itf){
    return zdl_close(itf->dl);
}

zinline int zplg_init(zplg_t *plg, zoperate cb, zvalue_t cb_hint, zvalue_t hint){
    zvalue_t values[2];
    values[0] = (zvalue_t)cb;
    values[1] = cb_hint;
    return plg->itf_init((zvalue_t)values, &plg->handle, hint);
}

zinline int zplg_fini(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_fini(plg->handle, out, hint);
}

zinline int zplg_run(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_run(plg->handle, out, hint);
}

zinline int zplg_stop(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_stop(plg->handle, out, hint);
}

zinline int zplg_operate(zplg_t *plg, zvalue_t *out, zvalue_t hint){
    return plg->itf_operate(plg->handle, out, hint);
}

zinline int zplg_ability(zplg_itf_t *itf, zvalue_t *out, zvalue_t hint){
    return itf->ability((zvalue_t)itf, out, hint);
}

#endif
