#include <zit/net/socket.h>
#include <zit/base/trace.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

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
  }
  ZERRC(ret);
  return(ret);
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
  }
  return(ret);
}

int zsend(zsock_t sock, const char *buf, int len, int flags){
  int ret;
  int sended;
  ret = 0;
  sended = 0;
  while(sended != len){
    ret = send(sock, buf+sended, len-sended, flags);
    if(ret >= 0){
      sended += ret;
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

int zconnectx(zsock_t sock, const char *host, uint16_t port, int listenq){
  int ret;
  zsockaddr_in addr;

  ret = zinet_addr(&addr, host, port);
  if(ret != ZOK){
    return(ret);
  }

  if(listenq > 0){
    int reuse = 1;
    zsock_nonblock(sock, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    zbind(sock, (ZSA*)&addr, sizeof(addr));
    ret = zlisten(sock, listenq);
  }else{
    ret = zconnect(sock, (ZSA*)&addr, sizeof(addr));
  }
  ZERRC(ret);
  return(ret);
}
