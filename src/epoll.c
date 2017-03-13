#include "export.h"
#include <zit/net/epoll.h>

#ifdef ZSYS_POSIX

#include <zit/base/trace.h>
#include <arpa/inet.h>

zsock_t zepoll_create(int size){
  zsock_t fd = epoll_create(size<256?256:size);
  if(-1 == fd){
    ZERRC(errno);
  }
  return(fd);
}

int zepoll_ctl(int epfd, int op, int fd, struct epoll_event *evt){
  int ret;
  ret = epoll_ctl(epfd, op, fd, evt);
  if(ZOK != ret){
    if(ret == EEXIST){
      ret = ZOK;
    }else{
      ret = ZFUN_FAIL;
    }
    ZERRC(errno);
  }
  return ret;
}

int zepoll_wait(int epfd, struct epoll_event *evt, int maxevents, int timeout){
  int ret;
  ret = epoll_wait(epfd, evt, maxevents, timeout);
  if(-1 == ret){
    ZERRC(errno);
  }
  return ret;
}
#endif
