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

write 500000 logs
# ZTRACE_0COPY 0
base log: 0.120668
not bkg: 0.450221
copy bkg: 1.482033
copy bkg: 1.195532

base log: 0.147761
not bkg: 0.460535
copy bkg: 1.130209
copy bkg: 1.226876

base log: 0.120387
not bkg: 0.474872
copy bkg: 1.116381
copy bkg: 0.950283

# ZTRACE_OCOPY 1

not bkg: 0.120973
not bkg: 0.510242
0-copy bkg: 2.705645
copy bkg: 2.246940
0-copy bkg: 2.282852

not bkg: 0.121270
not bkg: 0.468417
0-copy bkg: 1.647249
copy bkg: 2.403158
0-copy bkg: 1.488580

not bkg: 0.253457
not bkg: 0.490379
0-copy bkg: 1.431431
copy bkg: 1.619998
0-copy bkg: 1.051606

not bkg: 0.125118
not bkg: 0.472997
0-copy bkg: 1.352124
copy bkg: 1.115913
0-copy bkg: 1.451636

 */

#include "platform.h"
#include <zit/base/error.h>

ZC_BEGIN

ZAPI int zversion();
ZAPI const char* zsystem();
ZAPI const char* zhistory();
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
extern int g_ztrace_flag;

typedef int (*ztrace)(int level, void* user, const char* msg);
typedef int (*zgetbuf)(char **buf, int len);

ZAPI int ztrace_reg(ztrace fn, void* user);
ZAPI int ztrace_reg_0copy(ztrace fn, void *user, zgetbuf getfn);
ZAPI int ztrace_ctl(int flag);

ZAPI int zdbg(const char* msg, ...);
ZAPI int zmsg(const char* msg, ...);
ZAPI int zwar(const char* msg, ...);
ZAPI int zerr(const char* msg, ...);
ZAPI int zinf(const char* msg, ...);
ZAPI const char*  zstrerr(int code);

ZAPI int zdbgx(const char *title, int flag, const char* msg, ...);
ZAPI int zmsgx(const char *title, int flag, const char* msg, ...);
ZAPI int zwarx(const char *title, int flag, const char* msg, ...);
ZAPI int zerrx(const char *title, int flag, const char* msg, ...);
ZAPI int zinfx(const char *title, int flag, const char* msg, ...);


#define ZDBG(fmt, ...)       zdbg("[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZMSG(fmt, ...)       zmsg("[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZWAR(fmt, ...)       zwar("[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZERR(fmt, ...)       zerr("[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZINF(fmt, ...)       zinf("[ln:%04d fn:%s]\t" fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define ZERRC(x) zerr("[ln:%04d fn:%s]\t%s",__LINE__,__FUNCTION__,zstrerr(x))
#define ZERRCX(x) if( ZOK != (x) )ZERRC(x)
#define ZERRCXR(x) if( ZOK != (x) ){ZERRC(x); return(x);}
#define ZDUMP(x) zdbg("%s %s", #x, zstrerr(x))

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

#endif//_ZBASE_TRACE_H_
