#include "export.h"
#include <zit/net/socket.h>
#include <zit/base/trace.h>
#ifdef ZSYS_POSIX
#include <arpa/inet.h>
#else
#pragma comment(lib, "Ws2_32")
#endif
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int zsock_init(int v1, int v2){
#ifdef ZSYS_WINDOWS
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;
 
  wVersionRequested = MAKEWORD( 2, 2 );
 
  err = WSAStartup( wVersionRequested, &wsaData );
  if ( err != 0 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    return ZFUN_FAIL;
  }
 
  /* Confirm that the WinSock DLL supports 2.2.*/
  /* Note that if the DLL supports versions greater    */
  /* than 2.2 in addition to 2.2, it will still return */
  /* 2.2 in wVersion since that is the version we      */
  /* requested.                                        */
 
  if ( LOBYTE( wsaData.wVersion ) != 2 ||
       HIBYTE( wsaData.wVersion ) != 2 ) {
    /* Tell the user that we could not find a usable */
    /* WinSock DLL.                                  */
    WSACleanup( );
    return ZFUN_FAIL; 
  }
 
  /* The WinSock DLL is acceptable. Proceed. */
#endif
  return ZOK;
}
int zsock_fini(){
#ifdef ZSYS_WINDOWS
  WSACleanup();
#endif
  return ZOK;
}

zsock_t zsocket(int domain, int type, int protocol){
  zsock_t sock;
  sock = socket(domain, type, protocol);
  if(ZINVALID_SOCKET == sock){
    int err;
#ifdef ZSYS_WINDOWS
    err = WSAGetLastError();
#else
    err = errno;
#endif
    ZERRC(err);
  }else{
#if ZTRACE_SOCKET
    ZDBG("socket<%d>(domain<%d>,type<%d>,protocol<%d>", sock, domain, type, protocol);
#endif
    zsock_nonblock(sock, ZTRUE); // set default no block
  }
  return(sock);
}

int zsockclose(zsock_t sock){
  int ret;
#ifdef ZSYS_WINDOWS
  ret = closesocket(sock);
  ret = (SOCKET_ERROR == ret) ? WSAGetLastError() : ZOK;
#else //ZSYS_POSIX
  ret = close(sock);
  ret = (ret < 0)?errno:ZOK;
#endif
  ZERRC(ret);
  return(ret);
}

int zinet_addr(zsockaddr_in *addr, const char *host, uint16_t port){
  int ret;

  ret = ZOK;
  ZASSERT(!addr || !host);// || (port > 0 && port < 65535));
  memset(addr, 0, sizeof(zsockaddr_in));
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
#ifdef ZSYS_WINDOWS
  if(INADDR_NONE == (addr->sin_addr.S_un.S_addr = inet_addr(host))){
    ret = ZFUN_FAIL;
  }
#else
  if(inet_pton(AF_INET, host, &addr->sin_addr) <=0){
    ret = ZFUN_FAIL;
  }
#endif

#if ZTRACE_SOCKET
  zdbg("zinet(addr<%p>, host<%s>, port<%d>); %s", addr, host, port, zstrerr(ret));
#endif

  ZERRCX(ret);
  return(ret);
}

int zconnect(zsock_t sock, const ZSA *addr, int len){
  int ret;

  ret = ZOK;
  if(connect(sock, addr, len) < 0){
#ifdef ZSYS_WINDOWS
    ret = WSAGetLastError();
#else
    ret = errno;
#endif
    ZERRC(ret);
    ret = ZFUN_FAIL;
  }
#if ZTRACE_SOCKET
  else{
    ZERRC(ret);
  }
#endif
  return(ret);
}


static void sock_dump(const char *buf, int len){
  int i;
  int j;
  int offset;
  char msg[4096];
  
  if(len > 1024){
    len = 64;
  }
  i = offset = j = 0;
  while(i < len){
    offset += sprintf(msg+offset, "%02x ", (unsigned char)buf[i]);
    i++;
    j++;
    if(j == 32){
      offset += sprintf(msg+offset, "\n");
	  j = 0;
    }
  }
  zdbg("%s", msg);
}

int zrecv(zsock_t sock, char *buf, int len, int flags){
  int ret;
  int readed;
  if(0 > (readed = recv(sock, buf, len, flags))){
#ifdef ZSYS_WINDOWS
    ret = WSAGetLastError();
    if(WSAEWOULDBLOCK == ret || WSAEINTR == ret){
      ret = ZAGAIN;
    }
#else
    ret = errno;
    if(EAGAIN == ret || EWOULDBLOCK == ret || EINTR == ret){
      ret = ZAGAIN;
    }
#endif
    if(ret != ZAGAIN){
      ZERRC(ret);
      ret = ZFUN_FAIL;
    }
  }else{
    ret = readed;
    sock_dump(buf, ret);
  }
  return(ret);
}

int zsend(zsock_t sock, const char *buf, int len, int flags){
  int ret;
  int sended;
  ret = 0;
  sended = 0;
  while(sended != len){
#if ZTRACE_SOCKET
    //ZDBG("before send<sock:%d, buf:%p, len:%d, flags:%d>", sock, buf+sended, len-sended, flags);
    sock_dump(buf, len);
#endif
    ret = send(sock, buf+sended, len-sended, flags);
#if ZTRACE_SOCKET
    //ZDBG("after send<ret:%d>", ret);
#endif
    if(ret >= 0){
      sended += ret;
#if ZTRACE_SOCKET
      ZDBG("send<sended:%d, cur:%d, total:%d>", sended, ret, len);
#endif
    }else{
#ifdef ZSYS_WINDOWS
      ret = WSAGetLastError();
      if(WSAEWOULDBLOCK == ret || WSAEINTR == ret){
	ret = ZAGAIN;
      }
#else
      ret = errno;
      if(EAGAIN == ret || EWOULDBLOCK == ret || EINTR == ret){
	ret = ZAGAIN;
      }
#endif
      if(ZAGAIN == ret){
#ifdef ZTRACE_SOCKET
	ZDBG("try again...");
#endif
	continue;
      }else{
	ZERRC(ret);
	ret = ZFUN_FAIL;
	break;
      }
    }
  }
  if(ret != ZFUN_FAIL){
    ret = sended;
  }
  return(ret);
}

int zrecv_packet(zsock_t sock, char *buf, int maxlen, int *offset, int *len, char bitmask){
  int ret = ZFAIL;
  // ZASSERT(!buf || !offset || !len || !bitmask || !stuf);
  // remove old data and move new data to header
  //  zdbg("init offset<%d> len<%d>", *offset, *len);
  if(*offset != 0){
    if(*offset != *len){
      while(*(buf+(*offset)) != bitmask){
	//ZDBG("drop<%02x>", *(buf+(*offset)));
	++(*offset);
	if(*offset == *len){
	  *offset = *len = 0;
	  break;
	}
      }
      memmove(buf, buf + *offset, *len - *offset);
      *len = *len - *offset;
      *offset = 0;
      //------------------------------------
      // parse next packet
      if(*len != *offset){
	//	zdbg("fefore offset<%d> len<%d>", *offset, *len);
	while(++(*offset)){
	  if(*offset == *len){
	    *offset = *len = 0;
	    break;
	  }
	  if(*(buf+(*offset)) == bitmask){
	    ++(*offset);
	    ret = *offset;
	    //  zdbg("next offset<%d> len<%d>", *offset, *len);
	    return(ret);
	  }
	}
      }
    }else{
      *offset = *len = 0;
      // zdbg("reset offset<%d> len<%d>", *offset, *len);
    }
  }
  // start recv data, socket mask be nonblock
  ret = zrecv(sock, buf+(*len), maxlen-(*len), 0);
  if(ZAGAIN != ret && ZFUN_FAIL != ret && 0 != ret){
    *len += ret;
    while(++(*offset)){
      if(*(buf+(*offset)) == bitmask){
	++(*offset);
	ret = *offset;
      	//zdbg("111 offset<%d> len<%d>", *offset, *len);
	break;
      }
      if(*len == *offset){
	ret = ZAGAIN;
      	//zdbg("222 offset<%d> len<%d>", *offset, *len);
	break;
      }
    }
    //zdbg("333 offset<%d> len<%d>", *offset, *len);
  }
  //zdbg("end offset<%d> len<%d>", *offset, *len);
  return(ret);
}

int zselect(int maxfdp1, fd_set *read, fd_set *write, fd_set *except, struct timeval *timeout){
  int ret;
  ret = select(maxfdp1, read, write, except, timeout);
  if(ret < 0){
#ifdef ZSYS_WINDOWS
    ret = WSAGetLastError();
#else
    ret = errno;
#endif
    ZERRC(ret);
    ret = ZFUN_FAIL;
  }
  return(ret);
}

int zsock_nonblock(zsock_t sock, int noblock){
  int ret = ZOK;
#ifdef ZSYS_WINDOWS
  unsigned long ul = noblock;
  ret = ioctlsocket(sock, FIONBIO, &ul);
  if(SOCKET_ERROR == ret){
    ret = WSAGetLastError();
    ZERRC(ret);
    ret = ZFUN_FAIL;
  }
#else
  int val;
  val = fcntl(sock, F_GETFL, 0);
  if(1 == noblock){
    val |= O_NONBLOCK;
  }else{
    val &= (~O_NONBLOCK);
  }
  fcntl(sock, F_SETFL, val);
#endif
  return(ret);
}

int zbind(zsock_t sock, const ZSA *addr, int len){
  int ret;
  ret = bind(sock, addr, len);
#ifdef ZSYS_WINDOWS
  ret = (SOCKET_ERROR == ret) ? WSAGetLastError() : ZOK;
#else
  ret = (ret < 0) ? errno : ZOK;
#endif
  ZERRC(ret);
  return(ret);
}

int zlisten(zsock_t sock, int listenq){
  int ret;
  ret = listen(sock, listenq);
#ifdef ZSYS_WINDOWS
  ret = (SOCKET_ERROR == ret) ? WSAGetLastError() : ZOK;
#else
  ret = (ret < 0) ? errno : ZOK;
#endif
  ZERRC(ret);
  return(ret);
}

zsock_t zaccept(zsock_t sock, ZSA *addr, int *addrlen){
  int ret;
  zsock_t sk;
#ifdef ZSYS_WINDOWS
  sk = accept(sock, addr, addrlen);
  ret = (INVALID_SOCKET == sk) ? WSAGetLastError() : ZOK;
#else
  sk = accept(sock, addr, (socklen_t*)addrlen);
  ret = (sk < 0) ? errno : ZOK;
#endif
  ZERRC(ret);
  return(sk);
}

int zconnectx(zsock_t sock, const char *host, uint16_t port, int listenq, int timeout_ms){
  int ret;
  zsockaddr_in addr;

  ret = zinet_addr(&addr, host, port);
  if(ret != ZOK){
    return(ret);
  }

  if(listenq > 0){
    int reuse = 1;
    zsock_nonblock(sock, 0);
#ifdef ZSYS_WINDOWS
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));
#else
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
    zbind(sock, (ZSA*)&addr, sizeof(addr));
    ret = zlisten(sock, listenq);
  }else{
    if(-1 == timeout_ms){
      // block connect
#if ZTRACE_SOCKET
      ZDBG("start block connect...");
#endif
      zsock_nonblock(sock, 0);
      ret = zconnect(sock, (ZSA*)&addr, sizeof(addr));
      zsock_nonblock(sock, 1);
#if ZTRACE_SOCKET
      ZDBG("block connect end.");
#endif
      return ret;
    }
    // nonblock connect
    if(ZOK != (ret = zconnect(sock, (ZSA*)&addr, sizeof(addr)))){
      struct timeval tv;
      fd_set rset, wset;
      int error;
      socklen_t len;
      if(timeout_ms < 1000 || timeout_ms > 15000){
	timeout_ms = 4000;
      }
      tv.tv_sec = timeout_ms/1000;
      tv.tv_usec = (timeout_ms%1000)*1000;
      FD_ZERO(&rset);
      FD_SET(sock, &rset);
      FD_ZERO(&wset);
      FD_SET(sock, &wset);
#if ZTRACE_SOCKET
      ZDBG("conect timeout<sec:%d, usec:%d>", tv.tv_sec, tv.tv_usec);
#endif
      if((ret = zselect(sock+1, &rset, &wset, NULL, &tv)) > 0){
	ZDBG("select<%d>", ret);
	len = sizeof(error);
#ifdef ZSYS_WINDOWS
	getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error,&len);
#else
	getsockopt(sock, SOL_SOCKET, SO_ERROR, &error,&len);
#endif
	if(0 == error){
	  ret = ZOK;
	  ZDBG("connect ok");
	}else{
	  ret = ZFAIL;
	  ZDBG("connect fail");
	}
      }else{
	ret = ZTIMEOUT;
      }
    }
  }
  ZERRC(ret);
  return(ret);
}
