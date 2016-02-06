#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include "tbase.h"

void ztst_base(){
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
  
  zmsg("\ntest base end.\n");
}
