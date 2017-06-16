/**@file trace.c
 * @brief trace message
 */
#include "export.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef ZSYS_WINDOWS
#include <windows.h>
#include <lmerr.h>
#endif

#include <stdlib.h>
#include <time.h>

#define ZTRACE_0COPY 0

static zgetbuf g_ztrace_getbuf = NULL;
static ztrace g_ztrace = NULL;
static void* g_ztrace_user = NULL;
int g_ztrace_flag = 0x000000ff;

int ztrace_reg(ztrace fn, void* user) {
  g_ztrace = fn;
  g_ztrace_user = user;
  g_ztrace_getbuf = NULL;
  srand(time(NULL));
  return ZEOK;
}

int ztrace_reg_0copy(ztrace fn, void *user, zgetbuf getfn){
  g_ztrace = fn;
  g_ztrace_user = user;
  g_ztrace_getbuf = getfn;
  srand(time(NULL));
  return ZEOK;
}

int ztrace_state(ztrace *fn, void **user, int *flag){
  if(fn){
    *fn = g_ztrace;
  }

  if(user){
    *user =g_ztrace_user;
  }

  if(flag){
    *flag = g_ztrace_flag;
  }
  return ZOK;
}

int ztrace_ctl(int flag) {
  g_ztrace_flag = (flag & 0x000000ff);
  return ZEOK;
}

void zdump_mix(char *out, int size, const unsigned char *mix, int *len){
  int offset;
  int idx;
  unsigned char c;
  
  offset = 0;
  idx = 0;

  while(idx < *len){
    c = mix[idx];
    if( (c > 31 && c < 127) || (c > 7 && c < 14)){
      offset += sprintf(out+offset, "%c", mix[idx]);
    }else{
      offset += sprintf(out+offset, "[%02x]", mix[idx]);
    }
    if(offset + 5 > size){
      break; // safe down
    }
    ++idx;
    if(idx%64 == 0){
      offset += sprintf(out+offset, "\n");
    }

  }
  out[idx] = 0;
  *len = idx;
}

void zdump_bin(char *out, int size, const unsigned char *bin, int *len){
  int offset;
  int idx;
  offset = 0;
  idx = 0;

  while(idx< *len){
    offset += sprintf(out+offset, "%02x ", bin[idx]);
    if(offset + 5 > size){
      break; // safe down
    }
    ++idx;
    if(idx%32 == 0){
      offset += sprintf(out+offset, "\n");
    }
  }
  *len = idx;
}

int zrandin(int max){
  return (rand()%max);
}

ZAPI int zrandat(int begin, int end){
  return (begin + (rand()%(begin-end)));
}

int zdbgx(int flag, const char* msg, ...){
  if (g_ztrace && (flag & ZTRACE_FLAG_DBG)) {
    va_list arglist;
    int offset;
    //char buf[ZTRACE_BUF_SIZE];
#if ZTRACE_0COPY    
    char local_buf[ZTRACE_BUF_SIZE];
    char *buf;
    if(g_ztrace_getbuf){
      if(ZOK != g_ztrace_getbuf(&buf, ZTRACE_BUF_SIZE)){
	return ZMEM_INSUFFICIENT;
      }
    }else{
      buf = local_buf;
    }
#else
    char buf[ZTRACE_BUF_SIZE];
#endif
    zstr_systime_now(buf,ZTP_MILLISEC);
    offset = strlen(buf);
    offset += sprintf(buf+offset, " DBG:");
        
    va_start(arglist, msg);
    offset += vsnprintf(buf+offset, ZTRACE_BUF_SIZE-offset, msg, arglist);
    va_end(arglist);
    buf[offset++] = '\n';
    buf[offset] = 0;
    buf[offset+1] = (char)flag;
    g_ztrace(offset, g_ztrace_user, buf);
  }
  return ZEOK;
}
int zmsgx(int flag, const char* msg, ...){
  if (g_ztrace && (flag & ZTRACE_FLAG_MSG)) {
    va_list arglist;
    int offset;
    
#if ZTRACE_0COPY
    char local_buf[ZTRACE_BUF_SIZE];
    char *buf;
    if(g_ztrace_getbuf){
      if(ZOK != g_ztrace_getbuf(&buf, ZTRACE_BUF_SIZE)){
	return ZMEM_INSUFFICIENT;
      }
    }else{
      buf = local_buf;
    }
#else
    char buf[ZTRACE_BUF_SIZE];
#endif
    zstr_systime_now(buf,ZTP_MILLISEC);
    offset = strlen(buf);
    offset += sprintf(buf+offset, " MSG:");
    
    va_start(arglist, msg);
    offset += vsnprintf(buf+offset, ZTRACE_BUF_SIZE-offset, msg, arglist);
    va_end(arglist);
    buf[offset++] = '\n';
    buf[offset] = 0;
    g_ztrace(offset, g_ztrace_user, buf);    
  }
  return ZEOK;
}

int zwarx(int flag, const char* msg, ...){
  if (g_ztrace && (flag & ZTRACE_FLAG_WAR)) {
    va_list arglist;
    int offset;
    
#if ZTRACE_0COPY
    char local_buf[ZTRACE_BUF_SIZE];
    char *buf;
    if(g_ztrace_getbuf){
      if(ZOK != g_ztrace_getbuf(&buf, ZTRACE_BUF_SIZE)){
	return ZMEM_INSUFFICIENT;
      }
    }else{
      buf = local_buf;
    }
#else
    char buf[ZTRACE_BUF_SIZE];
#endif
    zstr_systime_now(buf,ZTP_MILLISEC);
    offset = strlen(buf);
    offset += sprintf(buf+offset, " WAR:");
    
    va_start(arglist, msg);
    offset += vsnprintf(buf+offset, ZTRACE_BUF_SIZE-offset, msg, arglist);
    va_end(arglist);
    buf[offset++] = '\n';
    buf[offset] = 0;
    g_ztrace(offset, g_ztrace_user, buf);    
  }
  return ZEOK;
}
int zerrx(int flag, const char* msg, ...){
  if (g_ztrace && (flag & ZTRACE_FLAG_ERR)) {
    va_list arglist;
    int offset;
    //char buf[ZTRACE_BUF_SIZE];
#if ZTRACE_0COPY    
    char local_buf[ZTRACE_BUF_SIZE];
    char *buf;
    if(g_ztrace_getbuf){
      if(ZOK != g_ztrace_getbuf(&buf, ZTRACE_BUF_SIZE)){
	return ZMEM_INSUFFICIENT;
      }
    }else{
      buf = local_buf;
    }
#else
    char buf[ZTRACE_BUF_SIZE];
#endif
    zstr_systime_now(buf,ZTP_MILLISEC);
    offset = strlen(buf);
    offset += sprintf(buf+offset, " ERR:");
    
    va_start(arglist, msg);
    offset += vsnprintf(buf+offset, ZTRACE_BUF_SIZE-offset, msg, arglist);
    va_end(arglist);
    buf[offset++] = '\n';
    buf[offset] = 0;
    g_ztrace(offset, g_ztrace_user, buf);    
  }
  return ZEOK;
}

int zinfx(int flag, const char* msg, ...){
  if (g_ztrace && (flag & ZTRACE_FLAG_INF)) {
    va_list arglist;
    int offset;
    //char buf[ZTRACE_BUF_SIZE];
#if ZTRACE_0COPY    
    char local_buf[ZTRACE_BUF_SIZE];
    char *buf;
    if(g_ztrace_getbuf){
      if(ZOK != g_ztrace_getbuf(&buf, ZTRACE_BUF_SIZE)){
	return ZMEM_INSUFFICIENT;
      }
    }else{
      buf = local_buf;
    }
#else
    char buf[ZTRACE_BUF_SIZE];
#endif    
    zstr_systime_now(buf,ZTP_MILLISEC);
    offset = strlen(buf);
    offset += sprintf(buf+offset, " INF:");
    
    va_start(arglist, msg);
    offset += vsnprintf(buf+offset, ZTRACE_BUF_SIZE-offset, msg, arglist);
    va_end(arglist);
    buf[offset++] = '\n';
    buf[offset] = 0;
    g_ztrace(offset, g_ztrace_user, buf);    
  }
  return ZEOK;
}

#ifdef ZSYS_WINDOWS
static int zwin_lasterror(int code, char* buf, int buflen) {
  int ret = ZEFUN_FAIL;
  HMODULE hModule = NULL;
  LPSTR MessageBuffer;
  DWORD dwBufferLength;
  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
  if (code >= NERR_BASE && code <= MAX_NERR) {
    hModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule != NULL)
      dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }

  if (dwBufferLength = FormatMessageA(dwFormatFlags, hModule, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&MessageBuffer, 0, NULL)) {
    MessageBuffer[dwBufferLength - 2] = 0;
    _snprintf(buf, buflen - 1, MessageBuffer);
    LocalFree(MessageBuffer);
    ret = ZEOK;
  }
  if (NULL != hModule) {
    FreeLibrary(hModule);
  }
  return ret;
}
#endif

const char *errmap[] = {
  "ZE0-ok",
  "ZE1-normall-error",
  "ZE2-memory-insufficient",
  "ZE3-call-function-fail",
  "ZE4-not-support",
  "ZE5-parameter-invalid",
  "ZE6-try-again",
  "ZE7-time-out",
  "ZE8-not-exist",
  "ZE9-not-init",
  "ZE10-status-invalid",
  "ZE11-memory-outofbounds",
  "ZE12-command-stop",
  "ZE13-compare-equal",
  "ZE14-compare-great",
  "ZE15-compare-little",
  "ZE16-command-restart",
  "ZE17-end-of-file",
  "ZE18-cast-type-fail",
  "ZE19-invalid-socket"
};

const char*  zstrerr(int code) {
  const char* msg = "unknownd error";
  static char lasterr[ZTRACE_BUF_SIZE] = { 0 };

  if ((code & ZEMASK) == ZEMASK) {
    code ^= ZEMASK;
    if(code <= ZE_END){
      msg = errmap[code];
    }//else unknown error
  }else if(ZEOK == code){
    msg = errmap[0];
  }else {
#ifdef ZSYS_POSIX
    snprintf(lasterr, ZTRACE_BUF_SIZE, "%s", strerror(code));
#else//ZSYS_WINDOWS
    zwin_lasterror(code, lasterr, ZTRACE_BUF_SIZE);
#endif
    msg = lasterr;
  }
  return msg;
}
