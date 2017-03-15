#ifndef _ZTRACE_NET_H_
#define _ZTRACE_NET_H_

#include <zit/base/platform.h>

ZC_BEGIN

ZAPI int ztrace_netctl(const char* hostname, int port);
ZAPI int ztrace_net(int level, void *user, const char *msg);

typedef int (*ztrace_netmsg)(const char* info, int len);
ZAPI int ztrace_netserver(int port, ztrace_netmsg fn);

ZAPI int ztrace_netstop();
ZC_END

/**@fn int ztrace_netctl(const char* hostname, int port)
 * @brief write <msg> to server, active connect for log realtime
 * @param const char* hostname [in] log server ip
 * @param int port [in] log server listen port
 * @note
 *  if log server closed or connection fail, just drop the <msg>.
 *  and not try reconnect to server.
 *  tcp connect.
 */

/**@fn int ztrace_net(int level, void *user, const char *msg)
 * @brief write <msg> to log server
 * @param int level [in] log level, defined in <zit/base/trace.h>
 * @param void *user [in] user data
 * @param const char * [in] log message
 */

/**@fn int ztrace_netstop()
 * @brief stop server and client
 */
#endif
