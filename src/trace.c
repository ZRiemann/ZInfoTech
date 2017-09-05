/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
/**
 * @file trace.c
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
ZAPI int g_ztrace_flag = 0x000000ff;

zerr_t ztrace_reg(ztrace fn, void* user) {
    g_ztrace = fn;
    g_ztrace_user = user;
    g_ztrace_getbuf = NULL;
    srand(time(NULL));
    return ZEOK;
}

zerr_t ztrace_reg_0copy(ztrace fn, void *user, zgetbuf getfn){
    g_ztrace = fn;
    g_ztrace_user = user;
    g_ztrace_getbuf = getfn;
    srand(time(NULL));
    return ZEOK;
}

zerr_t ztrace_state(ztrace *fn, void **user, int *flag){
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

zerr_t zdbgx(int flag, const char* msg, ...){
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
zerr_t zmsgx(int flag, const char* msg, ...){
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

zerr_t zwarx(int flag, const char* msg, ...){
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

zerr_t zerrx(int flag, const char* msg, ...){
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

zerr_t zinfx(int flag, const char* msg, ...){
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

ZAPI zerr_t ZOK = ZEOK;// 0 // ZError OK
ZAPI zerr_t ZFAIL = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
ZAPI zerr_t ZMEM_INSUFFICIENT = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
ZAPI zerr_t ZFUN_FAIL = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
ZAPI zerr_t ZNOT_SUPPORT = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
ZAPI zerr_t ZPARAM_INVALID = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
ZAPI zerr_t ZAGAIN = ZEAGAIN;// (ZEMASK |6) // try again
ZAPI zerr_t ZTIMEOUT = ZETIMEOUT;// (ZEMASK | 7) // operation time out
ZAPI zerr_t ZNOT_EXIST = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
ZAPI zerr_t ZNOT_INIT = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
ZAPI zerr_t ZSTATUS_INVALID = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
ZAPI zerr_t ZMEM_OUTOFBOUNDS = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
ZAPI zerr_t ZCMD_STOP = ZECMD_STOP;// (ZEMASK | 12) // return stop command
ZAPI zerr_t ZCMP_EQUAL = ZECMP_EQUAL;// (ZEMASK | 13) // compare equal
ZAPI zerr_t ZCMP_GREAT = ZECMP_GREAT;// (ZEMASK | 14) // compare great
ZAPI zerr_t ZCMP_LITTLE = ZECMP_LITTLE;// (ZEMASK | 15) // compare little
ZAPI zerr_t ZCMD_RESTART = ZECMD_RESTART; // 16
ZAPI zerr_t ZEOF = ZE_EOF; //17
ZAPI zerr_t ZCAST_FAIL = ZECAST_FAIL;// (ZMASK | 18) // cast type fail
ZAPI zerr_t ZSOCK = ZESOCK_INVALID;// (ZEMASK | 19) // invalid socket

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
