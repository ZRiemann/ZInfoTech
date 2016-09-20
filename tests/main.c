#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracering.h>
#include <zit/thread/jet.h>
#include "tbase.h"
//#include "tutility.h"
//#include "tthread.h"

int ztst_trace(int level, void* user, const char* msg){
  ztrace_console(level, user, msg);
  //ztrace_log(level, user, msg);
  return ZEOK;
}

int main(int argc, char** argv){  
  ztrace_logctl("ztest.log",0);
  //ztracering_init(ztst_trace, NULL);
  ztrace_reg(ztst_trace, NULL);
  //zjet_init();
  //  zjet_run();

#ifdef _ZTST_BASE_H_
  ztst_base();
#endif
#ifdef _ZTST_UTILITY_H_
  ztst_utility();
#endif
#ifdef _ZTST_THREAD_H_
    ztst_thread();
#endif

    //zjet_stop(0);
    //zjet_uninit();

    //  ztracering_uninit();
  ztrace_logctl(NULL,0); // close the log file.
  return 0;
}
