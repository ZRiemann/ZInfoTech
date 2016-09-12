#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/base/list.h>
#include "tbase.h"

static void ztst_list();
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
}

static void ztst_list(){
  zmsg("\n test zit list container...\n");
  zcontainer_t list;
  //  ZERRC(zlist_create(&list));
  zmsg("zlist_create(&list<%p>) %s", list, zstrerr(zlist_create(&list)));
  zmsg("zlist_destroy(list<%p>) %s", list, zstrerr(zlist_destroy(list, NULL)));
  zmsg("\n test zit list container end\n");
}
