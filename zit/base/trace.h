#ifndef _ZBASE_TRACE_H_
#define _ZBASE_TRACE_H_

/**@file zit/base/trace.h
 * @brief trace message
 */

#include "platform.h"
#include <zit/base/error.h>

#define ZTRACE_LEVEL_DBG 0
#define ZTRACE_LEVEL_MSG 1
#define ZTRACE_LEVEL_WAR 2
#define ZTRACE_LEVEL_ERR 3

typedef int (*ztrace)(int level, void* user, const char* msg);
ZEXP int ztrace_reg(ztrace fn, void* user);
ZEXP int ztrace_ctl(int flag);
ZEXP int zdbg(const char* msg, ...);
ZEXP int zmsg(const char* msg, ...);
ZEXP int zwar(const char* msg, ...);
ZEXP int zerr(const char* msg, ...);
ZEXP const char*  zstrerr(int code);

#define ZDBG(fmt, ...)       zdbg("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZMSG(fmt, ...)       zmsg("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZWAR(fmt, ...)       zwar("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)
#define ZERR(fmt, ...)       zerr("[ln:%04d fn:%s]\t"fmt,__LINE__,__FUNCTION__,##__VA_ARGS__)

#define ZERRC(x) zerr("[ln:%04d fn:%s]\t%s",__LINE__,__FUNCTION__,zstrerr(x))
#define ZERRCX(x) if( ZEOK != (x) )ZERRC(x)

#endif//_ZBASE_TRACE_H_
