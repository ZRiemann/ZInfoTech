/** @file zinc/basze/type.h
 *  @brief ZInfoTech types definitions
 *  @note zhuweiping 2016-01-04 found
 */
#ifndef _Z_TYPE_H_
#define _Z_TYPE_H_

#ifdef __cplusplus
//extern "C" {
#endif//__cplusplus

#include "platform.h"

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
  ZINIT = 0,
  ZRUN,
  ZSTOPING, // begin stop 
  ZSTOP,    // end stop
  ZUNINIT
}zstatus_t;
  
typedef void* zcontainer_t; // for all zit container
/* for all container value, caution in 64bit system:
 * int i; // 32bit
 * zvalue_t v; // 64bit
 * ZCONVERT(i,v); //use memset(v, i, sizeof(v)) instand of v = i;(warning)
 * ZCONVERT(v,i); //use memcpy(&i, v, sizeof(int)) instand of i = v;(warning)
 */
typedef void* zvalue_t;
// include <string.h> before use ZI2V()/ZV2I()
#define ZCONVERT(dest, src) do{memset(&(dest),0,sizeof(dest));memcpy(&(dest),&(src),(sizeof(src)<sizeof(dest)?sizeof(src):sizeof(dest)));}while(0)

typedef struct znode_t{
  zvalue_t value;
  struct znode_t* next;
  struct znode_t* prev;
}znod_t;

typedef int (*zcompare)(zvalue_t p1, zvalue_t p2); //return ZGREAT/ZEQUAL/ZLITTLE
typedef int (*zfree)(zvalue_t user, zvalue_t hint);
typedef int (*zact)(zvalue_t user, zvalue_t hint);

#define ZTSKMD_SEQUENCE 0
#define ZTSKMD_NORMAL 1

typedef struct ztask_t{
  int priority; ///< 0-low(idel) 1-normal 2-(above normal)hight priority queue
  int mode; ///< 0-sequence ; 1-normal
  int misid; ///< attach mission if sequence level
  zvalue_t user; ///< user data
  zvalue_t hint; ///< user hint
  zact act; ///< action task
  zfree free; ///< release user data
}ztsk_t;

ZEXP int zversion();
ZEXP const char* zsystem();
ZEXP const char* zhistory();

#ifdef __cplusplus
}
#endif//__cplusplus

/**@fn int zversion()
 * @brief get ZInfoTech version
 * @return version = 0x[char(major)char(minor)char(patch)] 
*/
/**@fn const char* system()
 * @brief get the system type description
 * @return system type descrition
 */
/**@fn const char* zhistory()
 * @brief get ZInfoTech version history
 * @return history string
 */

#endif//_Z_TYPE_H_
