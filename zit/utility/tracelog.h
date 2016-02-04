#ifndef _ZTRACE_LOG_H_
#define _ZTRACE_LOG_H_

#include <zit/base/platform.h>

#ifdef __cplusplus
extern "C"{
#endif

ZEXP int ztrace_logctl(const char* fname, int log_size);
ZEXP int ztrace_log(int level, const char* msg);

#ifdef __cplusplus
}
#endif
#endif
