#ifndef _ZIT_NET_SOCKET_H_
#define _ZIT_NET_SOCKET_H_
/**@file zit/net/socket.h
 * @brief socket wrapper
 * @note
 * @history
 */
#include <zit/base/type.h>
ZC_BEGIN
//===========================================
#ifdef ZSYS_WINDOWS
#include <windows.h>
#include <winsock2.h>

typedef SOCKET zsock_t;
typedef SOCKADDR_IN zsockaddr_in;
typedef struct sockaddr ZSA;
#define ZINVALID_SOCKET INVALID_SOCKET

//===========================================
#else //ZSYS_POSIX
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

typedef int zsock_t;
typedef struct sockaddr_in zsockaddr_in;
typedef struct sockaddr ZSA;
#define ZINVALID_SOCKET -1

#endif

/**@fn zsock_t zsocket(int domain, int type, int protocol)
 * @brief create an endpint for communication
 * @param int domain [in] AF_INET/INET6/UNIX/...
 * @param int type [in] SOCK_STREAM/DGRAM/SEQPACKET/RAW/RDM/PACKET
 * @param int protocol [in] default 0
 */
ZAPI zsock_t zsocket(int domain, int type, int protocol);
ZAPI int zinet_addr(zsockaddr_in *addr, const char *host, uint16_t port);
ZAPI int zconnect(zsock_t sock, const ZSA *addr, int len);
ZAPI int zrecv(sock_t sock, char *buf, int len, int flags);
ZAPI int zsend(sock_t sock, const char *buf, int len, int flags);

/**@fn int recv_packet(sock_t sock, char *buf, int maxlen, int* offset, int *len, char *bitmask, char *stuf)
 * @brief recv a packet
 */
ZAPI int recv_packet(sock_t sock, char *buf, int maxlen, int* offset, int *len, char bitmask, char *stuf);
ZC_END
#endif
