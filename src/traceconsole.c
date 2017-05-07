#include "export.h"
#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>

int ztrace_console(int level, void *user, const char* msg){
  printf("%s", msg);
  return ZOK;
}
