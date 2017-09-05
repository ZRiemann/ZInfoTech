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

ZC_BEGIN

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

ZAPI zerr_t zplg_scan(char path[512], const char *prefix, cbzftw ftw);
ZAPI zerr_t zplg_open(zplg_itf_t *itf, const char *filename);
ZAPI zerr_t zplg_ability(zplg_itf_t *itf, zvalue_t *out, zvalue_t hint);
ZAPI zerr_t zplg_close(zplg_itf_t *itf);
// plugin operates
ZAPI zerr_t zplg_init(zplg_t *plg, zoperate cb, zvalue_t cb_hint, zvalue_t hint);
ZAPI zerr_t zplg_fini(zplg_t *plg, zvalue_t *out, zvalue_t hint);
ZAPI zerr_t zplg_run(zplg_t *plg, zvalue_t *out, zvalue_t hint);
ZAPI zerr_t zplg_stop(zplg_t *plg, zvalue_t *out, zvalue_t hint);
ZAPI zerr_t zplg_operate(zplg_t *plg, zvalue_t *out, zvalue_t hint);


ZAPI zerr_t zplg_dummy(ZOP_ARG);
ZAPI zerr_t zplg_notsupport(ZOP_ARG);

ZC_END

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

#endif
