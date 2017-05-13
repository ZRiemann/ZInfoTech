#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracering.h>
#include <zit/utility/tracebkg.h>
#include <zit/thread/jet.h>
#include <zit/thread/rwlock.h>
#include <string.h>
#include <stdlib.h>
//#include "tbase.h"
//#include "tutility.h"
//#include "tthread.h"

static void bkglog(int argc, char ** argv);
static void rwlock(int argc, char ** argv);

int ztst_trace(int level, void* user, const char* msg){
  ztrace_console(level, user, msg);
  //ztrace_log(level, user, msg);
  return ZEOK;
}

int main(int argc, char** argv){  
  ztrace_logctl("ztest.log",80*1024*1024);
  //ztrace_reg(ztst_trace, NULL);
  ztrace_bkgctl(ztst_trace);
  ztrace_reg(ztrace_bkg, 0);
  //ztrace_reg_0copy(ztrace_0cpy_bkg, 0, ztrace_bkgbuf);

  zdbg("\n zit_test bkglog <lognum>"
       "\n zit_test rwlock");
  if(argc >= 2 && strcmp("bkglog", argv[1]) == 0){
    bkglog(argc, argv);
  }else if(argc >= 2 && strcmp("rwlock", argv[1]) == 0){
    rwlock(argc, argv);
  }else{
    zdbg("param error");
  }
#ifdef _ZTST_BASE_H_
  ztst_base();
#endif
#ifdef _ZTST_UTILITY_H_
  ztst_utility();
#endif
#ifdef _ZTST_THREAD_H_
  //ztst_thread();
#endif

  ztrace_bkgend();
  ztrace_logctl(NULL,0); // close the log file.
  return 0;
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
  ZERRC(zrwlock_init(&rwlock));
  ZERRC(zrwlock_rdlock(&rwlock));
  ZERRC(zrwlock_rdlock(&rwlock));
  ZERRC(zrwlock_rdlock(&rwlock));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_timedwrlock(&rwlock, 5));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_timedwrlock(&rwlock, 5));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_wrlock(&rwlock));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_tryrdlock(&rwlock));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_trywrlock(&rwlock));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_timedrdlock(&rwlock, 1));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_timedwrlock(&rwlock, 1));
  ZERRC(zrwlock_unlock(&rwlock));
  ZERRC(zrwlock_fini(&rwlock));
}
