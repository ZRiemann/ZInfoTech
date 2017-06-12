#ifndef _ZUTILITY_DAEMONIZE_H_
#define _ZUTILITY_DAEMONIZE_H_
/**@file zit/utility/daemonize.h
 * @brief daemonize
 * @note
 *  POSIX:
 *  1. umask();
 *  2. 
 */
#include <zit/base/platform.h>

ZC_BEGIN

ZAPI void daemoinize(const char *cmd);
ZAPI void already_running(const char *lockfile);
ZC_END

#endif
