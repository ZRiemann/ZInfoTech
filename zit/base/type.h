/** @file zinc/basze/type.h
 *  @brief ZInfoTech types definitions
 *  @note zhuweiping 2016-01-04 found
 */
#ifndef _Z_TYPE_H_
#define _Z_TYPE_H_

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
  
typedef void* zcontainer_t; // for all zit container
/* for all container value, caution in 64bit system:
 * int i; // 32bit
 * zvalue_t v; // 64bit
 * ZCONVERT(i,v); //use memset(v, i, sizeof(v)) instand of v = i;(warning)
 * ZCONVERT(v,i); //use memcpy(&i, v, sizeof(int)) instand of i = v;(warning)
 */
typedef void* zvalue_t;
typedef zvalue_t zatm_t;
typedef int32_t zspin_t;
typedef zvalue_t zptr_t;
typedef uint32_t zsize_t;

// include <string.h> before use ZI2V()/ZV2I()
#define ZCONVERT(dest, src) do{memset(&(dest),0,sizeof(dest));memcpy(&(dest),&(src),(sizeof(src)<sizeof(dest)?sizeof(src):sizeof(dest)));}while(0)

typedef union{
  zptr_t p;
  int i;
  //  double d;
}zany_t;// any type

typedef struct zpair_s{
  zany_t key;
  zvalue_t value;
  //zvalue_t hint;
}zpair_t;
// zany_t param[5]; // for param list
typedef struct znode_s{
  zvalue_t value;
  struct znode_s* next;
  struct znode_s* prev;
}znod_t;
/*
typedef int (*zcompare)(zvalue_t p1, zvalue_t p2); //return ZGREAT/ZEQUAL/ZLITTLE
typedef int (*zfree)(zvalue_t user, zvalue_t hint);
typedef int (*zact)(zvalue_t user, zvalue_t hint);
*/
typedef int (*zoperate)(zvalue_t in, zvalue_t *out, zvalue_t hint); // any operate
#define OPARG zvalue_t in,zvalue_t *out,zvalue_t hint
#define OPNULL NULL,NULL,NULL
#define OPIN(in) (zvalue_t)in,NULL,NULL
#define OPHINT(hint) NULL,NULL,(zvalue_t)hint
#define ZOP_ARG OPARG
#define ZOP_NULL OPNULL
#define ZOP_IN(in) (zvalue_t)in,NULL,NULL
#define ZOP_HINT(hint) NULL,NULL,(zvalue_t)hint

#define ZTSKMD_SEQUENCE 0
#define ZTSKMD_NORMAL 1
/*
typedef struct ztask_t{
  int priority; ///< 0-low(idel) 1-normal 2-(above normal)hight priority queue
  int mode; ///< 0-sequence ; 1-normal
  int misid; ///< attach mission if sequence level
  zvalue_t user; ///< user data
  zvalue_t hint; ///< user hint
  zact act; ///< action task(can not NULL)
  zfree free; ///< release user data(default NULL)
}ztsk_t;
*/

ZC_END

#endif//_Z_TYPE_H_
