#include "tutility.h"
#include <zit/base/trace.h>
#include <zit/utility/module.h>
#include <stdio.h>
#include <string.h>

static char zg_fix[256] = "fix.log";
static char zg_fix1[256] = "fix.log.log";
static void fixlogname();

void ztst_utility(){
  char path[256];
  char name[64];
  zmodule_name(path, name);
  ZDBG("\npath: %s;\nname: %s", path, name);
  fixlogname();
}

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
