#include "export.h"
#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>

int ztrace_console(int level, void *user, const char* msg){
  const char* szlevel;
  char now[64] = {0};

  switch(level){
    case ZTRACE_LEVEL_ERR:
      szlevel = " ERR:";
      break;
    case ZTRACE_LEVEL_WAR:
      szlevel = " WAR:";
      break;
    case ZTRACE_LEVEL_MSG:
      szlevel = " MSG:";
      break;
    case ZTRACE_LEVEL_DBG:
      szlevel = " DBG:";
      break;
    default:
      szlevel = " MSG:";
    break;
  }

  // CAUTION: NEED A LOCK FOR MULTITHREAD...
  zstr_systime_now(now,ZTP_MILLISEC);
  printf("%s%s %s\n",now,szlevel,msg);
  return ZEOK;
}
