#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracering.h>
#include <zit/utility/tracebkg.h>
#include <zit/thread/thread.h>
#include <zit/thread/rwlock.h>
#include <zit/base/dlfcn.h>
#include <zit/framework/plugin.h>

#include <string.h>
#include <stdlib.h>
#include <zit/base/time.h>

static zerr_t dump_int(ZOP_ARG);
static int g_run;
#include "tque1.h"
#include "tlist.h"
#include "tcontainer.h"

static void bkglog(int argc, char ** argv);
static void rwlock(int argc, char ** argv);
static void tzdl_open(int argc, char **argv);
static void tplugin(int argc, char ** argv);


#define ZTRACE_LOG 0
#define ZTRACE_BKG 0

int ztst_trace(int level, void* user, const char* msg){
    ztrace_console(level, user, msg);
    //ztrace_log(level, user, msg);
    return ZEOK;
}

int main(int argc, char** argv){
#if ZTRACE_LOG
    ztrace_logctl("ztest.log",80*1024*1024);
#endif
#if ZTRACE_BKG
    ztrace_bkgctl(ztst_trace);
    ztrace_reg(ztrace_bkg, 0);
#else
    ztrace_reg(ztrace_console, 0);
#endif
    //ztrace_reg_0copy(ztrace_0cpy_bkg, 0, ztrace_bkgbuf);

    zdbg("\n zit_test bkglog <lognum>"
         "\n zit_test rwlock"
         "\n zit_test zdl_open"
         "\n zit-test plugin"
        "\n zit_test queue_1r1w"
        "\n zit_test list_1r1w"
        "\n zit_test container");
    if(argc >= 2 && strcmp("bkglog", argv[1]) == 0){
        bkglog(argc, argv);
    }else if(argc >= 2 && strcmp("rwlock", argv[1]) == 0){
        rwlock(argc, argv);
    }else if(argc >= 2 && strcmp("zdl_open", argv[1]) == 0){
        tzdl_open(argc, argv);
    }else if(argc >= 2 && strcmp("plugin", argv[1]) == 0){
        tplugin(argc, argv);
    }else if(argc >= 2 && strcmp("queue_1r1w", argv[1]) == 0){
        tqueue_1r1w(argc, argv);
    }else if(argc >= 2 && strcmp("list_1r1w", argv[1]) == 0){
        list_1r1w(argc, argv);
    }else if(argc >= 2 && strcmp("container", argv[1]) == 0){
        tcontainer(argc, argv);
    }else{
        zdbg("param error");
    }
    zdbg("all testing down.");
#if ZTRACE_BKG
    ztrace_bkgend();
#endif

#if ZTRACE_LOG
    ztrace_logctl(NULL,0); // close the log file.
#endif
#if 0
	getchar();
#endif
    return 0;
}

static zerr_t dump_int(ZOP_ARG){
    static int max = 0;
    int i;
    ZCONVERT(i, in);
    printf("%d ", i);
    if(++max == 32){
        printf("\n");
    }
    return ZEOK;
}

static void tplugin(int argc, char ** argv){
    zplg_itf_t interface;
    zplg_t plugin;
    int ret;
    do{
        ret = zplg_open(&interface, "libtstso.so");
        ZERRCB(ret);
        plugin.itf = &interface;

        ret = zplg_init(&plugin, NULL, NULL, NULL);
        ZERRCB(ret);

        ret = zplg_run(&plugin, NULL, NULL);
        ZERRCB(ret);

        ret = zplg_operate(&plugin, NULL, NULL);
        ZERRCB(ret);

        ret = zplg_stop(&plugin, NULL, NULL);
        ZERRCB(ret);

        ret = zplg_fini(&plugin, NULL, NULL);
        ZERRCB(ret);

        ret = zplg_close(&interface);
    }while(0);

    ZERRC(ret);
}
typedef int (*ztadd)(int, int);

static void tzdl_open(int argc, char **argv){
    zdl_t dl;
    ztadd tadd;
    do{
#ifdef abcdefg
        zdbg("abcdefg ok");
#else
        zdbg("abcdefg not defined");
#endif

        dl = zdl_open("libtstso.so");
        ZERRCBX(NULL, dl);
        zdbg("zdl_open(\"libtstso.so\", RTLD_LAZY); OK");
        tadd = (ztadd)zdl_sym(dl, "tstso_add");
        ZERRCBX(NULL, tadd);
        zdbg("zdl_sym(dl, \"tstso_add\"); OK");
        tadd(11,22);
        zdbg("tadd(3,4) = %d", tadd(3,4));
        zdl_close(dl);

        //zdbg("tadd(3,4) = %d, after close", tadd(3,4)); // segmentation fault
    }while(0);
}
void bkglog(int argc, char ** argv){
    int i;
    void *tick;
    int sec, usec;
    int cnt;

    if(argc == 3){
        cnt = atoi(argv[2]);
        if(cnt < 0 || cnt > 1000000){
            cnt = 10000;
        }
    }else{
        cnt = 2000;
    }

    tick = ztick();
    FILE *pf;
    char pfbuf[256];
    int pflen;
    pf = fopen("make/bin/baselog", "w");
    if(pf){
        for(i = 0; i<cnt; i++){
            pflen = sprintf(pfbuf, "1234567890123456678986: %d\n", i);
            fwrite(pfbuf, 1, pflen, pf); 
        }
    }
    ztock(tick, &sec, &usec);
    if(pf)
    fclose(pf);
    printf("not bkg: %d.%06d\n", sec, usec);
    zsleepsec(3);
  
    ztrace_reg(ztrace_log, NULL);
    tick = ztick();
    for(i = 0; i<cnt; i++){
        zdbg("tick %d", i);
    }
    ztock(tick, &sec, &usec);
    printf("not bkg: %d.%06d\n", sec, usec);
    zsleepsec(3);

    /*
      ztrace_reg_0copy(ztrace_0cpy_bkg, 0, ztrace_bkgbuf);
      tick = ztick();
      for(i = 0; i<cnt; i++){
      zdbg("tick %d", i);
      }
      ztock(tick, &sec, &usec);
      printf("0-copy bkg: %d.%06d\n", sec, usec);
      zsleepsec(3);
    */
    ztrace_reg(ztrace_bkg, 0);
    tick = ztick();
    for(i = 0; i<cnt; i++){
        zdbg("tick %d", i);
    }
    ztock(tick, &sec, &usec);
    printf("copy bkg: %d.%06d\n", sec, usec);
    zsleepsec(3);

    ztrace_reg(ztrace_bkg, 0);
    tick = ztick();
    for(i = 0; i<cnt; i++){
        zdbg("tick %d", i);
    }
    ztock(tick, &sec, &usec);
    printf("copy bkg: %d.%06d\n", sec, usec);
    zsleepsec(3);

    /*
      ztrace_reg_0copy(ztrace_0cpy_bkg, 0, ztrace_bkgbuf);
      tick = ztick();
      for(i = 0; i<cnt; i++){
      zdbg("tick %d", i);
      }
      ztock(tick, &sec, &usec);
      printf("0-copy bkg: %d.%06d\n", sec, usec);
      zsleepsec(3);
    */
}


void rwlock(int argc, char ** argv){
    zrwlock_t rwlock;
    zerr_t ret;
    ret = ZEOK;
    zrwlock_init(&rwlock);
    zrwlock_rdlock(&rwlock);
    zrwlock_rdlock(&rwlock);
    zrwlock_rdlock(&rwlock);
    zrwlock_unlock(&rwlock, 1);
    zrwlock_unlock(&rwlock, 1);
    zrwlock_timedwrlock(&rwlock, 5, ret);
    zrwlock_unlock(&rwlock, 0);
    zrwlock_timedwrlock(&rwlock, 5, ret);
    zrwlock_unlock(&rwlock, 0);
    zrwlock_wrlock(&rwlock);
    zrwlock_unlock(&rwlock, 0);
    zrwlock_tryrdlock(&rwlock);
    zrwlock_unlock(&rwlock, 1);
    zrwlock_trywrlock(&rwlock);
    zrwlock_unlock(&rwlock, 0);
    zrwlock_timedrdlock(&rwlock, 1, ret);
    zrwlock_unlock(&rwlock, 1);
    zrwlock_timedwrlock(&rwlock, 1, ret);
    zrwlock_unlock(&rwlock, 0);
    zrwlock_fini(&rwlock);
    ZERRC(ret);
}
