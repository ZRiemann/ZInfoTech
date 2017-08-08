#ifndef _ZBASE_ERROR_H_
#define _ZBASE_ERROR_H_
/**@file zinc/base/error.h
 * @brief define ZInfoTech error numbers
 */
#include "platform.h"

#define ZEMASK 0xff000000
#define ZEOK 0 // ZError OK
#define ZEFAIL (ZEMASK | 1) // ZError FAILED
#define ZEMEM_INSUFFICIENT (ZEMASK | 2) // memory insufficient
#define ZEFUN_FAIL (ZEMASK | 3) // call function fail
#define ZENOT_SUPPORT (ZEMASK | 4) // not support
#define ZEPARAM_INVALID (ZEMASK | 5) // parameter invalid
#define ZEAGAIN (ZEMASK |6) // try again
#define ZETIMEOUT (ZEMASK | 7) // operation time out
#define ZENOT_EXIST (ZEMASK | 8) // Device/object not exist
#define ZENOT_INIT (ZEMASK | 9)  // Device not init
#define ZESTATUS_INVALID (ZEMASK | 10)  // device status invalid
#define ZEMEM_OUTOFBOUNDS (ZEMASK | 11) // memory out of bounds
#define ZECMD_STOP (ZEMASK | 12) // return stop command
#define ZECMP_EQUAL (ZEMASK | 13) // compare equal
#define ZECMP_GREAT (ZEMASK | 14) // compare great
#define ZECMP_LITTLE (ZEMASK | 15) // compare little
#define ZECMD_RESTART (ZEMASK | 16) // return restart command
#define ZE_EOF (ZEMASK | 17) // end of file
#define ZECAST_FAIL (ZEMASK | 18) // cast type fail
#define ZESOCK_INVALID (ZEMASK | 19) // invalid socket
#define ZE_END (19)

ZEXTERN int ZOK; // = ZEOK;// 0 // ZError OK
ZEXTERN int ZFAIL;// = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
ZEXTERN int ZMEM_INSUFFICIENT;// = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
ZEXTERN int ZFUN_FAIL;// = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
ZEXTERN int ZNOT_SUPPORT;// = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
ZEXTERN int ZPARAM_INVALID;// = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
ZEXTERN int ZAGAIN;// = ZEAGAIN;// (ZEMASK |6) // try again
ZEXTERN int ZTIMEOUT;// = ZETIMEOUT;// (ZEMASK | 7) // operation time out
ZEXTERN int ZNOT_EXIST;// = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
ZEXTERN int ZNOT_INIT;// = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
ZEXTERN int ZSTATUS_INVALID;// = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
ZEXTERN int ZMEM_OUTOFBOUNDS;// = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
ZEXTERN int ZCMD_STOP;// = ZECMD_STOP;// (ZEMASK | 12) // return stop command
ZEXTERN int ZCMP_EQUAL;// (ZEMASK | 13) // compare equal
ZEXTERN int ZCMP_GREAT;// (ZEMASK | 14) // compare great
ZEXTERN int ZCMP_LITTLE;// (ZEMASK | 15) // compare little
ZEXTERN int ZCMD_RESTART;
ZEXTERN int ZEOF;
ZEXTERN int ZCAST_FAIL; // (ZMASK | 18) // cast type fail
ZEXTERN int ZSOCK; // (ZEMASK | 19) // invalid socket


#define ZASSERTR(x) if(ZOK != (x))return(x)
#define ZASSERTB(x) if(x)break
#define ZASSERTBC(x,c) if(x){ret = c; break;}
#define ZASSERTC(x,c) if(x)return(c)
#define ZASSERT(x) if(x)return(ZEPARAM_INVALID)
#define ZASSERTX(x) if(x)return

#endif
