#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/base/list.h>
#include <zit/base/queue.h>
#include <zit/base/container.h>
#include "tbase.h"
#include <stdlib.h>
#include <string.h>

static void ztst_list();
static void ztst_que();
static void ztst_container();
void ztst_base(){
  int i,j;
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

  ztst_list();
  ztst_que();
  ztst_container();
}


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
