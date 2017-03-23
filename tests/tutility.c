#include "tutility.h"
#include <zit/base/trace.h>
#include <zit/utility/module.h>
#include <zit/utility/convert.h>
#include <stdio.h>
#include <string.h>

#define ZTST_CONVERT 1
#define ZTST_FIXLOGNAME 0

#if ZTST_CONVERT
void ztst_convert(){
  char buf[64];
  char out[64];
  int len;

  len = strlen(buf);
  sprintf(buf,"浙A12345");
  zconv_u2g(1, buf, strlen(buf), out, &len);
  ZDBG("%s",buf);
  ZDBG("%s",out);
}
#endif

#if ZTST_FIXLOGNAME
static char zg_fix[256] = "fix.log";
static char zg_fix1[256] = "fix.log.log";
static void fixlogname();

void fixlogname(){
  printf("fixlogname ...\n");
  if((NULL == strchr(zg_fix, '\\')) || (NULL == strchr(zg_fix, '/'))){
    char path[256];
    if(ZEOK == zmodule_name(path, NULL)){
      printf("path: %s\n", path);
      strcat(path,"/");
      strcat(path, zg_fix);
      strcpy(zg_fix, path);
      strcat(path, ".log");
      strcpy(zg_fix1, path);
      printf("zg_fix: %s\n",zg_fix);
      printf("zg_fix1: %s\n", zg_fix1);
    }
  }
}
#endif

void ztst_utility(){
#if ZTST_CONVERT
  ztst_convert();
#endif
#if ZTST_FIXLOGNAME
  fixlogname();
#endif
}
