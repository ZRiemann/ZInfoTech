#include "export.h"
#include <zit/base/error.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/net/socket.h>
#include <zit/utility/tracenet.h>
#include <string.h>

static zsock_t zg_logfd = ZINVALID_SOCKET;
static ztrace_netmsg zg_netfn = NULL;

static int znet_send(const char* buf, int len){
  int ret;
  int sended;
  
  ret = ZOK;
  sended = 0;
  if(ZINVALID_SOCKET == zg_logfd){
    return ret;
  }

  while(sended != len){
    ret = send(zg_logfd, buf+sended, len-sended, 0);

    if(ret >= 0){
      sended += ret;
    }else{
#ifdef ZSYS_WINDOWS
      ret = WSAGetLastError();
      if(WSAEWOULDBLOCK == ret || WSAEINTR == ret){
	ret =ZAGAIN;
      }
#else
      ret = errno;
      if(EAGAIN == ret || EWOULDBLOCK == ret || EINTR == ret){
	ret = ZAGAIN;
      }
#endif
      if(ZAGAIN == ret){
	continue;
      }else{
#ifdef ZSYS_WINDOWS
	ret = WSAGetLastError();
	closesocket(zg_logfd);
#else
	close(zg_logfd);
#endif
	zg_logfd = ZINVALID_SOCKET;
	ret = ZFUN_FAIL;
	break;
      }
    }
  }
  if(ret != ZFUN_FAIL){
    ret = sended;
  }
  return ret;
}

int ztrace_netctl(const char* hostname, int port){
  int ret;
  zsockaddr_in addr;
#ifdef ZSYS_WINDOWS
  unsigned long ul;

  ul = 1;
#endif

  ret = ZFUN_FAIL;
  
  zg_logfd = socket(AF_INET, SOCK_STREAM, 0);
  if(ZINVALID_SOCKET == zg_logfd){
    return ZFUN_FAIL;
  }
  do{
    memset(&addr, 0, sizeof(zsockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef ZSYS_WINDOWS
    if(INETADDR_NONE == (addr.sin_addr.S_un.S_addr = inet_addr(hostname))){
      break;;
    }
    ret = ioctlsocket(zg_logfd, FIONBIO, &ul);
    if(SOCKET_ERROR == ret){
      break;
    }
#else
    if(inet_pton(AF_INET, hostname, &addr.sin_addr) <= 0){
      break;
    }
    ret = fcntl(zg_logfd, F_GETFL, 0);
    ret |= O_NONBLOCK;
    fcntl(zg_logfd, F_SETFL, ret);
    ret = ZOK;
#endif
  
    ret = connect(zg_logfd, (ZSA*)&addr, sizeof(addr));
  
    if(ZOK != ret){
      struct timeval tv;
      fd_set rset, wset;
      int error;
      socklen_t len;
      tv.tv_sec = 3;
      tv.tv_usec = 0;
      FD_ZERO(&rset);
      FD_SET(zg_logfd, &rset);
      FD_ZERO(&wset);
      FD_SET(zg_logfd, &wset);
      
      ret = select(zg_logfd+1, &rset, &wset, NULL, &tv);
      if(ret > 0){
	len = sizeof(error);
	getsockopt(zg_logfd, SOL_SOCKET, SO_ERROR, &error, &len);
	if(0 == error){
	  ret = ZOK;
	}
      }else{
	ret = ZTIMEOUT;
      }
    }
  }while(0);
  if(ZOK != ret){
#ifdef ZSYS_WINDOWS
    ret = WSAGetLastError();
    closesocket(zg_logfd);
#else
    close(zg_logfd);
#endif
    zg_logfd = ZINVALID_SOCKET;
    ret = ZFUN_FAIL;    
  }
  return ret;
}

int ztrace_net(int level, void *user, const char *msg){
  if(ZINVALID_SOCKET != zg_logfd){
    char now[64];
    const char *szlevel;

    zstr_systime_now(now, ZTP_MILLISEC);
    znet_send(now, strlen(now));
    
    switch(level){
    case ZTRACE_LEVEL_ERR:
      szlevel = " ERR:";
      break;
    case ZTRACE_LEVEL_WAR:
      szlevel = " WAR:";
      break;
    case ZTRACE_LEVEL_MSG:
      szlevel = " MSG:";
      break;
    case ZTRACE_LEVEL_DBG:
      szlevel = " DBG:";
      break;
    default:
      szlevel = " MSG:";
    }
    znet_send(szlevel, strlen(szlevel));

    znet_send(msg, strlen(msg));
  }
  return ZOK;
}

int ztrace_netserver(int port, ztrace_netmsg fn){
  int ret;

  ret = ZOK;
  zg_netfn = fn;

  return ret;
}
/**@file tracenet.c
 * @brief send msg to log server
 * @note
 *  not use zsocket api, because it may trace log cause dead loop.
 */
