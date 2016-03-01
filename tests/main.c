#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
#include <zit/thread/jet.h>
//#include "tbase.h"
//#include "tutility.h"
#include "tthread.h"

// trace at back ground thread pool use ringbuf(zjet:mis[0])
#include <zit/base/ringbuf.h>
#include <string.h>
zring_t zg_tracering;
ztsk_t zg_tracetsk;
char zg_tracebuf[4096] = {0};
int ztraceact(zvalue_t user, zvalue_t hint){
  // do actual trace
#if 1
  char c = 0;
  int len = 1;
  int len1 = 4096;
  
  if((ZEOK == zring_read((zring_t*)user, &c, &len) && (ZEOK == zring_strread((zring_t*)user, (char*)hint, &len1)))){
    c -= 'a';
    printf("+++");
    ztrace_console((int)c, (char*)hint);
    //ztrace_log((int)c, (char*)hint);
  }
  return ZEOK;
#endif
}

int ztst_tracexinit(){
  zring_init(&zg_tracering, 4096*100);
  zg_tracetsk.user = &zg_tracering;
  zg_tracetsk.mode = ZTSKMD_SEQUENCE;
  zg_tracetsk.misid = 0;
  zg_tracetsk.user = &zg_tracering;
  zg_tracetsk.hint = zg_tracebuf;
  zg_tracetsk.act = ztraceact;
  zg_tracetsk.free = NULL;
  return ZEOK;
}

int ztst_tracexuninit(){
  zring_uninit(&zg_tracering);
  return ZEOK;
}

int ztst_tracex(int level, void* user, const char* msg){
  char c = (char)level + 'a';
  int len = 1;
  int len1 = strlen(msg)+1;
#if 1
  if(ZRUN == zjet_getstatus()){
    // trace background
    zmutex_lock(&(zg_tracering.mtx)); // lock for multi thread write ring
    if(ZEOK == zring_write(&zg_tracering, &c, &len) && (ZEOK == zring_strwrite(&zg_tracering, msg, &len1))){
      zjet_assign(&zg_tracetsk);
    }
    zmutex_unlock(&(zg_tracering.mtx));
  }else{
    // trace directory
    printf("---");
    ztrace_console(level, msg);
    //ztrace_log(level, msg);
  }  
#endif
  return ZEOK;
}


// trace directory
int ztst_trace(int level, void* user, const char* msg){
  ztrace_console(level, msg);
  //ztrace_log(level, msg);
  return ZEOK;
}

int main(int argc, char** argv){
  int i = 0;
  
  ztrace_logctl("ztest.log",0);
  ztrace_reg(ztst_tracex, NULL);
  //ztrace_reg(ztst_trace, NULL);
  ztst_tracexinit();

  zjet_init();
  zjet_run();
#ifdef _ZTST_BASE_H_
  ztst_base();
#endif
#ifdef _ZTST_UTILITY_H_
  ztst_utility();
#endif
#ifdef _ZTST_THREAD_H_
  //  ztst_thread();
#endif
  zsleepsec(1);
  for(i=0; i<10; i++){
    zdbg("aaaaaaaaaaaaaaaaaaa");
  }
  zsleepsec(1);
  zjet_stop(0);
  zjet_uninit();

  for(i=0; i<3; i++){
    zdbg("bbbbbbbbbbbbbbbb");
  }
  ztrace_logctl(NULL,0); // close the log file.
  ztst_tracexuninit();
  return 0;
}
