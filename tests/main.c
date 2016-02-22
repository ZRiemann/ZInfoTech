#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
//#include "tbase.h"
//#include "tutility.h"
#include "tthread.h"

// trace directory
int ztst_trace(int level, void* user, const char* msg){
  ztrace_console(level, msg);
  //ztrace_log(level, msg);
  return ZEOK;
}
// trace at back ground thread pool(zjet)
int ztst_tracexinit(){
  
}
int ztst_tracexuninit(){

}
int ztst_tracex(int level, void* user, const char* msg){
  
}

int main(int argc, char** argv){
  ztrace_reg(ztst_trace, NULL);
  ztrace_logctl("ztest.log",0);
#ifdef _ZTST_BASE_H_
  ztst_base();
#endif
#ifdef _ZTST_UTILITY_H_
  ztst_utility();
#endif
#ifdef _ZTST_THREAD_H_
  ztst_thread();
#endif
  ztrace_logctl(NULL,0); // close the log file.
  return 0;
}
