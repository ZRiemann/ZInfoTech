#ifndef _ZTRACE_LOG_H_
#define _ZTRACE_LOG_H_

#include <zit/base/platform.h>

ZC_BEGIN

ZAPI int ztrace_logctl(const char* fname, int log_size);
ZAPI int ztrace_log(int level, void *user, const char* msg);

ZC_END

/** @fn int ztrace_logctl(const char* fname, int log_size)
 *  @brief control trace log file name, max size and *close log file*
 *  @param const char* fname [in] log file name
 *  @param int log_size [in] set log max size
 *  @note
 *   ztrace_logctl(NULL, 0); // if open then close log file
 */
/** @fn int trace_log(int level, void *user const char* msg);
 *  @brief write <msg> to log file
 *  @param int level [in] log level, defined in <zit/base/trace.h>
 *  @param void *user [in] user data
 *  @param const char * [in] log message
 */
#endif
