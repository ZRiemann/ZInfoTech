#include <stdio.h>
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include "tbase.h"

int ztst_trace(int level, void* user, const char* msg){
  printf("level: %d msg: %s\n",level, msg);
  return ZEOK;
}

void ztst_base(){
  int v = zversion();
  ztrace_reg(ztst_trace, NULL);
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
}
