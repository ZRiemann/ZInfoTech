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

/** @fn int ztrace_logctl(const char* fname, int log_size)
 *  @brief control trace log file name, max size and *close log file*
 *  @param const char* fname [in] log file name
 *  @param int log_size [in] set log max size
 */
/** @fn int trace_log(int level, const char* msg);
 *  @brief write <msg> to log file
 *  @param int level [in] log level, defined in <zit/base/trace.h>
 */
#endif
