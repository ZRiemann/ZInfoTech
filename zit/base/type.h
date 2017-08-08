#ifndef _Z_TYPE_H_
#define _Z_TYPE_H_
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
#include <inttypes.h>
#endif//ZSYS_WINDOWS

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif//__cplusplus
#endif//NULL

#ifndef ZTRUE
#define ZTRUE 1
#endif
#ifndef ZFALSE
#define ZFALSE 0
#endif

#ifndef ZMAX_PATH
#define ZMAX_PATH 256
#endif

#ifndef ZFRONT
#define ZFRONT 0
#endif
#ifndef ZBACK
#define ZBACK 1
#endif

#ifndef ZEQUAL
#define ZEQUAL 0
#endif
#ifndef ZGREAT
#define ZGREAT 1
#endif
#ifndef ZLITTLE
#define ZLITTLE -1
#endif

typedef enum{
    ZUNINIT = 0,
    ZINIT,
    ZRUN,
    ZSTOPING, // begin stop 
    ZSTOP    // end stop
}zstatus_t;

#define ZSTAT_FINI 0
#define ZSTAT_INIT 1
#define ZSTAT_RUN 2
#define ZSTAT_STOP 3
#define ZSTAT_PENDING 4 // not in any status while switch status

typedef void* zvalue_t;
typedef zvalue_t zptr_t;
typedef zvalue_t zcontainer_t; // for all zit container

typedef uint32_t zsize_t;
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

#ifdef ZSYS_WINDOWS
#define zmem_align(alignment, size) _aligned_malloc(size,alignment)
#define zmem_align64(size) _aligned_malloc(size, 64)
#define zmem_align_free(ptr) _aligned_free(ptr)
#else // ZSYS_POSIX
#define zmem_align(alignment, size) memalign(alignment, size)
#define zmem_align64(size) memalign(64, size)
#define zmem_align_free(ptr) free(ptr)
#endif

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

typedef struct zpair_s{
    zany_t key;
    zvalue_t value;
    //zvalue_t hint;
}zpair_t;

typedef struct znode_s{
    zvalue_t value;
    struct znode_s* next;
    struct znode_s* prev;
    char net[]; // parents/children(network)
}znod_t;

#define ZCHUNK_SIZE 4096
typedef struct zchunk_s{
    zvalue_t value[ZCHUNK_SIZE];
    struct zchunk_s *next;
    struct zchunk_s *prev;
}zchunk_t;

#define OPARG zvalue_t in,zvalue_t *out,zvalue_t hint
#define OPNULL NULL,NULL,NULL
#define OPIN(in) (zvalue_t)in,NULL,NULL
#define OPHINT(hint) NULL,NULL,(zvalue_t)hint
#define ZOP_ARG OPARG
#define ZOP_NULL OPNULL
#define ZOP_IN(in) (zvalue_t)in,NULL,NULL
#define ZOP_HINT(hint) NULL,NULL,(zvalue_t)hint

#if 0
#define zprint(fmt, ...) printf("[ln:%4d fn:%s]\t" fmt, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define zprint(fmt, ...)
#endif

#define ZUSE_STATISTIC 0

#define ZTSKMD_SEQUENCE 0
#define ZTSKMD_NORMAL 1

ZC_END

#endif//_Z_TYPE_H_
