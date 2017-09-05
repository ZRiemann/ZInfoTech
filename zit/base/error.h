#ifndef _ZBASE_ERROR_H_
#define _ZBASE_ERROR_H_
/**@file zinc/base/error.h
 * @brief define ZInfoTech error numbers
 */
#include <zit/base/type.h>

ZC_BEGIN

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

ZEXTERN zerr_t ZOK;
ZEXTERN zerr_t ZFAIL;
ZEXTERN zerr_t ZMEM_INSUFFICIENT;
ZEXTERN zerr_t ZFUN_FAIL;
ZEXTERN zerr_t ZNOT_SUPPORT;
ZEXTERN zerr_t ZPARAM_INVALID;
ZEXTERN zerr_t ZAGAIN;
ZEXTERN zerr_t ZTIMEOUT;
ZEXTERN zerr_t ZNOT_EXIST;
ZEXTERN zerr_t ZNOT_INIT;
ZEXTERN zerr_t ZSTATUS_INVALID;
ZEXTERN zerr_t ZMEM_OUTOFBOUNDS;
ZEXTERN zerr_t ZCMD_STOP;
ZEXTERN zerr_t ZCMP_EQUAL;
ZEXTERN zerr_t ZCMP_GREAT;
ZEXTERN zerr_t ZCMP_LITTLE;
ZEXTERN zerr_t ZCMD_RESTART;
ZEXTERN zerr_t ZEOF;
ZEXTERN zerr_t ZCAST_FAIL;
ZEXTERN zerr_t ZSOCK;


#define ZASSERTR(x) if(ZOK != (x))return(x)
#define ZASSERTB(x) if(x)break
#define ZASSERTBC(x,c) if(x){ret = c; break;}
#define ZASSERTC(x,c) if(x)return(c)
#define ZASSERT(x) if(x)return(ZEPARAM_INVALID)
#define ZASSERTX(x) if(x)return

ZC_END

#endif
