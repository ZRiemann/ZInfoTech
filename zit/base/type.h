#ifndef _Z_TYPE_H_
#define _Z_TYPE_H_
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
 * @file zit/base/type.h
 * @brief ZInfoTech types definitions
 * @note zhuweiping 2016-01-04 found
 */

#include "platform.h"

ZC_BEGIN

#ifdef ZSYS_WINDOWS
#include "inttypes.h"
#elif defined(ZSYS_POSIX)
#include <sys/types.h> // size_t
#include <inttypes.h>
#endif//ZSYS_WINDOWS

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif//__cplusplus
#endif//NULL

#define ZTRUE 1
#define ZFALSE 0
#define ZMAX_PATH 256
#define ZFRONT 0
#define ZBACK 1
#define ZEQUAL 0
#define ZGREAT 1
#define ZLITTLE -1

#define ZSTAT_FINI 0
#define ZSTAT_INIT 1
#define ZSTAT_RUN 2
#define ZSTAT_STOP 3
#define ZSTAT_PENDING 4 // not in any status while switch status
#define ZSTAT_BUSY ZSTAT_PENDING

#define zst_offset(st, member) ((long)&((st*)0)->member)
#define zmember2st(st, member, pm) ((char*)pm - zst_offset(st, member))
#define zst2member(st, member, pst) ((char*)pm + zst_offset(st, member))

typedef void* zvalue_t;
typedef zvalue_t zptr_t;
typedef zvalue_t zcontainer_t; // for all zit container
typedef uint32_t zotype_t;
typedef uint32_t zoid_t;
typedef int32_t zerr_t;
typedef zerr_t (*zoperate)(zvalue_t in, zvalue_t *out, zvalue_t hint); // any operate

/* for all container value, caution in 64bit system:
 * int i; // 32bit
 * zvalue_t v; // 64bit
 * ZCONVERT(i,v); //use memset(v, i, sizeof(v)) instand of v = i;(warning)
 * ZCONVERT(v,i); //use memcpy(&i, v, sizeof(int)) instand of i = v;(warning)
 */

#define ZCONVERT(dest, src) do{memset(&(dest),0,sizeof(dest));memcpy(&(dest),&(src),(sizeof(src)<sizeof(dest)?sizeof(src):sizeof(dest)));}while(0)

typedef union{
    zptr_t p;
    int i;
    double d;
    float f;
    int32_t i32;
    uint32_t u32;
    int64_t i64;
    uint64_t u64;
}zany_t;// any base type
#define ZANY_CMP(any1, any2) (any1.i64 - any2.i64)

typedef struct zpair_s{
    zany_t key;
    zany_t value;
}zpair_t;

#define ZOP_ARG zvalue_t in,zvalue_t *out,zvalue_t hint
#define ZOP_NULL NULL,NULL,NULL
#define ZOP_IN(in) (zvalue_t)in,NULL,NULL
#define ZOP_HINT(hint) NULL,NULL,(zvalue_t)hint

#if 0
#include <stdio.h>
#define zprint(fmt, ...) printf("[ln:%4d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define zprint(fmt, ...)
#endif

ZC_END

#endif//_Z_TYPE_H_
