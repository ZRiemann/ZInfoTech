#ifndef _ZBASE_TRACE_H_
#define _ZBASE_TRACE_H_

/**@file zit/base/trace.h
 * @brief trace message
 */

#include "platform.h"
#include <zit/base/error.h>

ZC_BEGIN

ZAPI int zversion();
ZAPI const char* zsystem();
ZAPI const char* zhistory();

#define ZTRACE_BUF_SIZE 4096
#define ZTRACE_LEVEL_DBG 0
#define ZTRACE_LEVEL_MSG 1
#define ZTRACE_LEVEL_WAR 2
#define ZTRACE_LEVEL_ERR 3

typedef int (*ztrace)(int level, void* user, const char* msg);
ZAPI int ztrace_reg(ztrace fn, void* user);
ZAPI int ztrace_ctl(int flag);
ZAPI int zdbg(const char* msg, ...);
ZAPI int zmsg(const char* msg, ...);
ZAPI int zwar(const char* msg, ...);
ZAPI int zerr(const char* msg, ...);
ZAPI const char*  zstrerr(int code);

#define ZDBG(fmt, ...)       zdbg("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZMSG(fmt, ...)       zmsg("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZWAR(fmt, ...)       zwar("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZERR(fmt, ...)       zerr("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define ZERRC(x) zerr("[ln:%04d fn:%s]\t%s",__LINE__,__FUNCTION__,zstrerr(x))
#define ZERRCX(x) if( ZOK != (x) )ZERRC(x)
#define ZERRCXR(x) if( ZOK != (x) ){ZERC(x);return(x));
#define ZDUMP(x) zdbg("%s %s", #x, zstrerr(x))

// control module trace
#define ZTRACE_MUTEX 0
#define ZTRACE_SEM 0
#define ZTRACE_QUE 0
#define ZTRACE_RING 0 // set 0 if use trace background
#define ZTRACE_SOCKET 1 
#define ZTRACE_FRAMEWORK 1
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
