#include "export.h"
#include <zit/base/error.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/thread/thread.h>
#include <zit/net/socket.h>
#include <zit/utility/tracenet.h>
#include <string.h>

static zsock_t zg_logfd = ZINVALID_SOCKET;
static ztrace_netmsg zg_netfn = NULL;
static int zg_tracenet_state = 0;

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
    now[0] = '\n';
    znet_send(now, 1);
  }
  return ZOK;
}

int ztrace_netstop(){
  if(zg_logfd != ZINVALID_SOCKET){
#ifdef ZSYS_WINDOWS
    closesocket(zg_logfd);
#else
    close(zg_logfd);
#endif
    zg_logfd = ZINVALID_SOCKET;    
  }
  if(zg_tracenet_state == 1){
    int cnt = 3;
    zg_tracenet_state = 2;
    while(--cnt != 0){
      zsleepsec(1);
      if(0 == zg_tracenet_state){
	break;
      }
    }  
  }
  return ZOK;
}
//==================================================

zthr_ret_t ZCALL zproc_tracenet(void* param){
  zsock_t listenfd;
  zsock_t acceptfd;
  struct timeval tv;
  fd_set set;
  int ret;
  char buf[1024];
  socklen_t len;
  int error;

  listenfd = zsocket(AF_INET, SOCK_STREAM, 0);
  if(ZINVALID_SOCKET == listenfd){
    return (zthr_ret_t)0;
  }
  acceptfd = ZINVALID_SOCKET;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  zconnectx(listenfd, "0.0.0.0", (int)param, 5, 3000);

  while(zg_tracenet_state != 2){
    // wait connect...
    FD_ZERO(&set);
    FD_SET(listenfd, &set);
    ret = select(listenfd+1, &set, NULL, NULL, &tv);
    if(ret > 0){
      len = sizeof(error);
      getsockopt(listenfd, SOL_SOCKET, SO_ERROR, &error, &len);
      if(0 == error){
	acceptfd = zaccept(listenfd, (ZSA*)NULL, NULL);
	if(acceptfd == ZINVALID_SOCKET){
	  continue;
	}
	zsock_nonblock(acceptfd, ZTRUE);
	while(zg_tracenet_state != 2){
	  // reading messages...
	  FD_ZERO(&set);
	  FD_SET(acceptfd, &set);
	  if(1 == zselect(acceptfd+1, &set, NULL, NULL, &tv)){
	    ret = zrecv(acceptfd, buf, 1023, 0);
	    if(ret > 0 && ret <= 1023){
	      if(zg_netfn){
		buf[ret] = 0;
		zg_netfn(buf, ret);
	      }
	    }else if(ret == 0){
#ifdef ZSYS_WINDOWS
	      closesocket(acceptfd);
#else
	      close(acceptfd);
#endif
	      acceptfd = ZINVALID_SOCKET;
	      break; // goto wait connect...
	    }
	  }
	}
      }else{
	//...
      }
    }
  }
  zg_tracenet_state = 0;
#ifdef ZSYS_WINDOWS
  closesocket(listenfd);
  if(ZINVALID_SOCKET == acceptfd){
    closesocket(acceptfd);
  }
#else
  close(listenfd);
  if(ZINVALID_SOCKET == acceptfd){
    close(acceptfd);
  }
#endif
  listenfd = ZINVALID_SOCKET;      
  acceptfd = ZINVALID_SOCKET;
  return (zthr_ret_t)0;
}

int ztrace_netserver(int port, ztrace_netmsg fn){
  int ret;
  zthr_id_t tracenet_id;
  ret = ZOK;
  zg_netfn = fn;
  
  if(zg_tracenet_state != 0){
    return ZOK;
  }
  if(ZOK == zthread_create(&tracenet_id, zproc_tracenet, (void*)port)){
    zthread_detach(&tracenet_id);
    zg_tracenet_state = 1;
  }
  
  return ret;
}
/**@file tracenet.c
 * @brief send msg to log server
 * @note
 *  not use zsocket api, because it may trace log cause dead loop.
 */
