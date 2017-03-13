#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/base/list.h>
#include <zit/base/queue.h>
#include <zit/base/container.h>
#include <zit/framework/task.h>
#include <zit/thread/thread_def.h>
#include <zit/base/filesys.h>
#include "tbase.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ZTST_FTW 0
#define ZTST_DIR 1
//static void ztst_list();
//static void ztst_que();
//static void ztst_container();
static void ztst_ftw();
static void ztst_dir();
//static void ztst_framework();
void ztst_base(){
  ztst_ftw();
  ztst_dir();
  /* int i,j;
  char buf[256];
  int v = zversion();
  zmsg("=====================================\ntesting base...\n");
  zmsg("\nZInfoTech:\nversion: %06x\nsystem: %s\nhistory: %s\n",v, zsystem(),zhistory());
  zmsg(zstrerr(ZEOK));
  zmsg(zstrerr(ZEFAIL));
  ZERRC(ZEOK);
  ZERRC(ZEFAIL);
  ZERRCX(ZEOK);
  ZERRCX(ZEFAIL);
  ZDBG("test ZDBG");
  ZMSG("test ZMSG");
  ZWAR("test ZWAR");
  ZERR("test ZERR");
  zdbg("test zdbg()");
  zmsg("test zmsg()");
  zwar("test zwar()");
  zerr("test zerr()");

  zdbg("now precision second:\t %s", zstr_systime_now(buf, ZTP_SEC));
  zdbg("now precision millisecond:%s", zstr_systime_now(buf, ZTP_MILLISEC));
  zdbg("now precision microsecond:%s", zstr_systime_now(buf, ZTP_MICROSEC));

  for(i=1; i<=ZE_END; i++){
    j = i | ZEMASK;
    ZERRCX(j);
  }
  zmsg("\ntest base end.\n");
*/
  //ztst_list();
  //ztst_que();
  //  ztst_container();
  //ztst_framework();
}
static void ztst_dir(){
#if ZTST_DIR
  const char *dir = "tstdir";
  char pathname[512];
  zmkdir(dir,0x755);
  zmkdir(dir,0x755);
  sprintf(pathname, ".");
  zftw_nr(pathname, print_zftw, 0);
  zrmdir(dir);
  sprintf(pathname, ".");
  zftw_nr(pathname, print_zftw, 0);
#endif
}
static void ztst_ftw(){
#if ZTST_FTW
  char pathname[512];
  sprintf(pathname, "/nfsroot");
  zftw(pathname, print_zftw);
  zftw_nr(pathname, print_zftw);
#endif
}
#if 0
//====================================================
#define ZTSK_TYPE_COUNTER (ZOBJ_TYPE_USER + 1)
static int  check[0x100000];
static int op_counter(OPARG){
  ztsk_t *tsk;
  int i;
  tsk = (ztsk_t*)in;
  i = tsk->param[0].i;
  ++(check[i]);
  if(i >= tsk->param[1].i){
    zsem_post((zsem_t*)tsk->hint);
  }
  return(ZOK);
}

static void ztst_framework(){
  ztsk_svr_t *svr;
  zmis_t *mis;
  int max_task;
  int i;
  zsem_t sem_down;
  int ret;
  ztsk_t *tsk;
  int check_ok;
  int check_fail;
  zsem_init(&sem_down,0);
  max_task = 0x100000;
  mis = (zmis_t*)malloc(sizeof(zmis_t));
  ZDUMP(zmis_init(OPIN(mis)));
  //mis->mode = ZMIS_MODE_CONCURRENT; //1M/1220ms/4thread  1050ms/1thread
  mis->mode = ZMIS_MODE_SERIAL; // 1M/970ms/4thread 1280ms/1thread
  zmis_attach_op(mis, ZTSK_TYPE_COUNTER, op_counter);
 
  ZDUMP(ztsk_svr_create(&svr));
  ZDUMP(svr->dev.init(OPIN(svr)));
  svr->worknum = 4;
  ZDUMP(svr->dev.run(OPIN(svr)));
  ZDUMP(ztsk_svr_observer(svr, mis, ZTSK_TYPE_COUNTER));
  // post max_task tasks
  for(i=0; i<=max_task; i++){
    ret = ztsk_svr_gettask(svr, &tsk, ZTSK_TYPE_COUNTER);
    if(ZOK != ret){
      zerr("get task[%d] failed<%s>", i, zstrerr(ret));
      break;
    }
    tsk->param[0].i = i;
    tsk->param[1].i = max_task;
    tsk->hint = (zvalue_t*)&sem_down;
    ztsk_svr_post(svr, tsk);
    //if((i & 0X000003FF) == 0){
    //  zdbg("%d task posted", i);
    //}
  }
  zdbg("all task post down...");
  // wait all task down
  zsem_wait(&sem_down, ZINFINITE);
  zdbg("all task down.");
  check_ok = 0;
  check_fail = 0;
  for(i = 0; i<=max_task; i++){
    if(check[i]!=1){
      //zdbg("check[%d]=%d", i, check[i]);
      check_fail++;
    }else{
      //zdbg("%d check ok", i);
      check_ok++;
    }
  }
  
  zdbg("all task working down. <ok:%d, fail:%d>", check_ok, check_fail);
  
  ZDUMP(svr->dev.stop(OPIN(svr)));
  ZDUMP(svr->dev.fini(OPIN(svr)));
  ZDUMP(ztsk_svr_destroy(svr));
  ZDUMP(zmis_fini((zvalue_t)mis, NULL, NULL));
  free(mis);
  zsem_uninit(&sem_down);
}

//=====================================================
static int zprint(OPARG){
  int* i = (int*)hint;
  int d;

  ZCONVERT(d,in);
  zdbg("[%03d] %d", ++*i, d);
  return(ZOK);
}

static void ztst_container(){
  zcontainer_t cont;
  int i;
  zvalue_t v;
  zmsg("\n test zit container...\n");
  ZDUMP(zcontainer_create(&cont, ZCONTAINER_LIST));
  i=1; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zcontainer_pushfront(cont,v));
  i = 0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  ZDUMP(zcontainer_popback(cont, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  i = 0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v));
  i=0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v));
  i = 1; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i=0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_destroy(cont,NULL));

  ZDUMP(zcontainer_create(&cont, ZCONTAINER_CKQUEUE));
  i=1; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zcontainer_pushfront(cont,v));
  i = 0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  ZDUMP(zcontainer_popback(cont, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  i = 0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v));
  i=0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_pop(cont, &v));
  i = 1; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zcontainer_push(cont,v));
  i=0; zcontainer_foreach((zcontainer_t*)cont, zprint,(zvalue_t)&i);
  ZDUMP(zcontainer_destroy(cont,NULL));
  
  zmsg("\n test zit container end\n");
}
static void ztst_que(){
  zcontainer_t list;
  int i;
  zvalue_t v;
  zmsg("\n test zit que container...\n");
  ZDUMP(zque_create(&list));
  i=1; ZCONVERT(v,i); ZDUMP(zque_push(list,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zque_push(list,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zque_pushfront(list,v));
  i = 0; zque_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);
  ZDUMP(zque_pop(list, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  ZDUMP(zque_popback(list, &v)); ZCONVERT(i,v); zdbg("pop<%d>", i);
  i = 0; zque_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);
  ZDUMP(zque_pop(list, &v));
  i=0; zque_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);
  ZDUMP(zque_pop(list, &v));
  i = 1; ZCONVERT(v,i); ZDUMP(zque_push(list,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zque_push(list,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zque_push(list,v));
  i=0; zque_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);

  ZDUMP(zque_destroy(list,NULL));
  zmsg("\n test zit que container end\n");
}
static void ztst_list(){
  zcontainer_t list;
  int i;
  zvalue_t v;
  
  zmsg("\n test zit list container...\n");
  ZDUMP(zlist_create(&list));
  i=1;
  ZCONVERT(v,i);
  ZDUMP(zlist_push(list,v));
  i = 2;
  ZCONVERT(v,i);
  ZDUMP(zlist_push(list,v));
  i = 3;
  ZCONVERT(v,i);
  ZDUMP(zlist_pushfront(list,v));
  i = 0;
  zlist_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);
  ZDUMP(zlist_pop(list, &v));
  ZCONVERT(i,v);
  zdbg("pop<%d>", i);
  ZDUMP(zlist_popback(list, &v));
  ZCONVERT(i,v);
  zdbg("pop<%d>", i);
  i = 0;
  zlist_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);

  ZDUMP(zlist_pop(list, &v));
  i=0; zlist_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);
  ZDUMP(zlist_pop(list, &v));

  i = 1; ZCONVERT(v,i); ZDUMP(zlist_push(list,v));
  i = 2; ZCONVERT(v,i); ZDUMP(zlist_push(list,v));
  i = 3; ZCONVERT(v,i); ZDUMP(zlist_push(list,v));
  i=0; zlist_foreach((zcontainer_t*)list, zprint,(zvalue_t)&i);

  ZDUMP(zlist_destroy(list,NULL));
  zmsg("\n test zit list container end\n");
}
#endif
