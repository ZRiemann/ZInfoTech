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
#include "export.h"
#include <zit/base/trace.h>
#include <zit/utility/convert.h>
#include <string.h>
#ifdef ZSYS_POSIX
#include <iconv.h>
#include <errno.h>
#else
#include <Windows.h>
#endif

int zconv_u2g(int isu2g, char *in, int inlen, char *out, int *outlen){
#ifdef ZSYS_POSIX
    iconv_t cd;
    int ret;
    const char *uset = "utf-8";
    const char *gset = "gb2312";

    ret = ZOK;

    if(isu2g){
        cd = iconv_open(gset, uset);
    }else{
        cd = iconv_open(uset, gset);
    }

    if(cd == (iconv_t)-1){
        ZERRC(errno);
        return ZFUN_FAIL;
    }
    memset(out, 0, *outlen);
    if(-1 == iconv(cd, &in, (size_t*)&inlen, &out, (size_t*)outlen)){
        ZERRC(errno);
        ret = ZFUN_FAIL;
    }

    iconv_close(cd);
    return ret;
#else
    return ZNOT_SUPPORT;
#endif
}



