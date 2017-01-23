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
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
ZAPI int zsockclose(zsock_t sock);
// active connect
ZAPI int zinet_addr(zsockaddr_in *addr, const char *host, uint16_t port);
ZAPI int zconnect(zsock_t sock, const ZSA *addr, int len);
ZAPI int zrecv(zsock_t sock, char *buf, int len, int flags);
ZAPI int zsend(zsock_t sock, const char *buf, int len, int flags);
// passive connect
ZAPI int zbind(zsock_t sock, const ZSA *addr, int len);
ZAPI int zlisten(zsock_t sock, int listenq); // listenq=1024
ZAPI zsock_t zaccept(zsock_t sock, ZSA *addr, int *addrlen);
// select() -1 infinit 0 no wait >0 wait ms 
ZAPI int zselect(int maxfdp1, fd_set *read, fd_set *write, fd_set *except, struct timeval *timeout);
// get/setoption()
ZAPI int zsock_nonblock(zsock_t sock, int noblock);
/**@fn int recv_packet(sock_t sock, char *buf, int maxlen, int* offset, int *len, char *bitmask)
 * @brief recv a packet
 * @return ZOK - sock closed
 * @return ZAGAIN - recv ok but need more
 * @return ZFUN_FAIL - call system API failed
 * @return ret>0 - packet len
 * @note
 * buf - packet begin
 * offset - packet end + 1
 * len - next packet data
 */
ZAPI int zrecv_packet(zsock_t sock, char *buf, int maxlen, int* offset, int *len, char bitmask);
/**@fn int zconnectx(zsock_t sock, const char *host, uint 16_t port, int listenq)
 * @brief listenq <= 0 active connect listenq > 0 passive connect
 */
ZAPI int zconnectx(zsock_t sock, const char *host, uint16_t port, int listenq, int timeout_ms);

#define ZSOCKCLOSE(sock) do{zsockclose(sock); (sock)=ZINVALID_SOCKET;}while(0)
ZC_END
#endif
