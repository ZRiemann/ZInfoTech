#include "export.h"
#include <zit/utility/module.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <string.h>

#ifdef ZSYS_POSIX
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#else // ZSYS_WINDOWS
#include <windows.h>
#endif
int zmodule_name(char* path, char* name){
  int ret = ZEOK;
  char buf[1024];
  char c;
  char* dest;
  
#ifdef ZSYS_POSIX
  ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf)-1);
  if(-1 == len){
    ret = errno;
  }else{
    buf[len] = '\0';
    dest = strrchr(buf,'/');
  }  
#else//ZSYS_WINDOWS
  int len = GetModuleFileNameA(NULL,buf,1024);
  if( 0 == len ){
    ret = GetLastError();
  }
  dest = strrchr(buf,'\\');
#endif
  if( ZEOK != ret){
    return ret;
  }
  if(NULL != dest){
    *(dest) = '\0';
  }else{
    ret = ZENOT_EXIST;
    return ret;
  }
  if(NULL != path){
    strcpy(path,buf);
  }
  if(NULL != name){
    strcpy(name,dest+1);
  }

  //ZERRC(ret); // CAUTION: cause dead loop in tracelog.c
  return ret;
}
