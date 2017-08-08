#ifndef _ZBASE_TRACE_H_
#define _ZBASE_TRACE_H_

/**@file zit/base/trace.h
 * @brief trace message
 * @note
comment config:
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/tracebkg.h>

int zit_trace(int level, void* user, const char* msg){
  //ztrace_console(level, user, msg);
  ztrace_log(level, user, msg); // FILE* not thread safe, need background write.
  return ZEOK;
}

int main(int argc, char** argv){  
  ztrace_logctl("zit.log",64*1024*1024);
  ztrace_bkgctl(zit_trace);
  ztrace_reg(ztrace_bkg, 0); // thread safe
  //...
  ztrace_bkgend();
  ztrace_logctl(NULL,0); // close the log file.
}
*/

#include "platform.h"
#include <zit/base/error.h>

ZC_BEGIN

ZAPI int zversion();
ZAPI const char* zsystem();
ZAPI const char* zhistory(char buf[1024]);
ZAPI void zdump_mix(char *out, int size, const unsigned char *mix, int *len);
ZAPI void zdump_bin(char *out, int size, const unsigned char *bin, int *len);
ZAPI int zrandin(int max); // rand 0~max-1
ZAPI int zrandat(int begin, int end); // rand begin~end-1

#define ZTRACE_BUF_SIZE 4096
#define ZTRACE_LEVEL_DBG 0
#define ZTRACE_LEVEL_MSG 1
#define ZTRACE_LEVEL_WAR 2
#define ZTRACE_LEVEL_ERR 3
#define ZTRACE_LEVEL_INF 4 // user business logic infomation

#define ZTRACE_FLAG_DBG 0x01
#define ZTRACE_FLAG_MSG 0x02
#define ZTRACE_FLAG_WAR 0x04
#define ZTRACE_FLAG_ERR 0x08
#define ZTRACE_FLAG_INF 0x10
ZEXTERN int g_ztrace_flag;

typedef int (*ztrace)(int level, void* user, const char* msg);
typedef int (*zgetbuf)(char **buf, int len);

ZAPI int ztrace_reg(ztrace fn, void* user);
ZAPI int ztrace_reg_0copy(ztrace fn, void *user, zgetbuf getfn);
ZAPI int ztrace_state(ztrace *fn, void **user, int *flag);
ZAPI int ztrace_ctl(int flag);

ZAPI const char*  zstrerr(int code);

ZAPI int zdbgx(int flag, const char* msg, ...);
ZAPI int zmsgx(int flag, const char* msg, ...);
ZAPI int zwarx(int flag, const char* msg, ...);
ZAPI int zerrx(int flag, const char* msg, ...);
ZAPI int zinfx(int flag, const char* msg, ...);

#define zdbg(fmt, ...) zdbgx(g_ztrace_flag, fmt, ##__VA_ARGS__)
#define zmsg(fmt, ...) zmsgx(g_ztrace_flag, fmt, ##__VA_ARGS__)
#define zwar(fmt, ...) zwarx(g_ztrace_flag, fmt, ##__VA_ARGS__)
#define zerr(fmt, ...) zerrx(g_ztrace_flag, fmt, ##__VA_ARGS__)
#define zinf(fmt, ...) zinfx(g_ztrace_flag, fmt, ##__VA_ARGS__)

//#define zprint(fmt, ...) zdbgx(g_ztrace_flag, fmt, ##__VA_ARGS__)


#define ZDBG(fmt, ...)       zdbgx(g_ztrace_flag, "[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZMSG(fmt, ...)       zmsgx(g_ztrace_flag, "[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZWAR(fmt, ...)       zwarx(g_ztrace_flag, "[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZERR(fmt, ...)       zerrx(g_ztrace_flag, "[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZINF(fmt, ...)       zinfx(g_ztrace_flag, "[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define ZDBGC(x) zdbgx(g_ztrace_flag, "[ln:%04d fn:%s]\t%s",__LINE__,__FUNCTION__,zstrerr(x)) // for buglevel
#define ZERRC(x) zerrx(g_ztrace_flag, "[ln:%04d fn:%s]\t%s",__LINE__,__FUNCTION__,zstrerr(x))
#define ZERRCB(x) if( ZOK != (x)){ZERRC(x); break;}
#define ZERRCBX(c, x) if( (c) == (x)){ZERRC(ZFAIL); break;}
#define ZERRCX(x) if( ZOK != (x) )ZERRC(x)
#define ZERRCXR(x) if( ZOK != (x) ){ZERRC(x); return(x);}
#define ZDUMP(x) zdbgx(g_ztrace_flag, "%s %s", #x, zstrerr(x))

// control module trace
#define ZTRACE_MUTEX 0
#define ZTRACE_SEM 0
#define ZTRACE_QUE 0
#define ZTRACE_RING 0 // set 0 if use trace background
#define ZTRACE_SOCKET 1 
#define ZTRACE_FRAMEWORK 0
#define ZTRACE_LIST 0
ZC_END

/**@fn int zversion()
 * @brief get ZInfoTech version
 * @return version = 0x[char(major)char(minor)char(patch)] 
*/
/**@fn const char* system()
 * @brief get the system type description
 * @return system type descrition
 */
/**@fn const char* zhistory()
 * @brief get ZInfoTech version history
 * @return history string
 */
/*
write 500000 logs to file
$ make/bin/zit_test bkglog 500000
not bkg: 0.121014
not bkg: 0.439808
copy bkg: 0.845756
copy bkg: 0.650955
$ make/bin/zit_test bkglog 500000
not bkg: 0.137679
not bkg: 0.476832
copy bkg: 0.819397
copy bkg: 0.664261
$ make/bin/zit_test bkglog 500000
not bkg: 0.120228
not bkg: 0.452248
copy bkg: 0.847605
copy bkg: 0.745492
*/
#endif//_ZBASE_TRACE_H_
