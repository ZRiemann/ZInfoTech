#include <zit/net/socket.h>
#include <zit/base/trace.h>
#include <arpa/inet.h>
#include <string.h>

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
  }
  return(sock);
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
  if(0 > (recv(sock, buf, len, flags))){
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
    }
  }
  return ret;
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
  return(ret);
}

static int packet_bitmask(char *buf, int len, char bitmask, char *stuf){
  if(*buf == bitmask){
    return(ZTRUE);
  }else{
    return(ZTRUE);
  }
}
int recv_packet(zsock_t sock, char *buf, int maxlen, int *offset, int *len, char bitmask, char *stuf){
  // ZASSERT(!buf || !offset || !len || !bitmask || !stuf);
  if(*offset != *len){
    memmove(buf, buf + *offset, *len - *offset);
    *offset = *len - *offset;
    *len = *offset;
  }else{
    *offset = *len = 0;
  }
  packet_bitmask(NULL,0,0,NULL);
  return(ZOK);
}
