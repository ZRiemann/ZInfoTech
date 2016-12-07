#ifndef _ZBASE_ERROR_H_
#define _ZBASE_ERROR_H_
/**@file zinc/base/error.h
 * @brief define ZInfoTech error numbers
 */

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
#define ZE_END (17)

extern const int ZOK; // = ZEOK;// 0 // ZError OK
extern const int ZFAIL;// = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
extern const int ZMEM_INSUFFICIENT;// = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
extern const int ZFUN_FAIL;// = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
extern const int ZNOT_SUPPORT;// = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
extern const int ZPARAM_INVALID;// = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
extern const int ZAGAIN;// = ZEAGAIN;// (ZEMASK |6) // try again
extern const int ZTIMEOUT;// = ZETIMEOUT;// (ZEMASK | 7) // operation time out
extern const int ZNOT_EXIST;// = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
extern const int ZNOT_INIT;// = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
extern const int ZSTATUS_INVALID;// = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
extern const int ZMEM_OUTOFBOUNDS;// = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
extern const int ZCMD_STOP;// = ZECMD_STOP;// (ZEMASK | 12) // return stop command
extern const int ZCMP_EQUAL;// (ZEMASK | 13) // compare equal
extern const int ZCMP_GREAT;// (ZEMASK | 14) // compare great
extern const int ZCMP_LITTLE;// (ZEMASK | 15) // compare little
extern const int ZCMD_RESTART;
extern const int ZEOF;

#define ZASSERTR(x) if(ZOK != (x))return(x)
#define ZASSERTB(x) if(x)break
#define ZASSERTBC(x,c) if(x){ret = c; break;}
#define ZASSERTC(x,c) if(x)return(c)
#define ZASSERT(x) if(x)return(ZEPARAM_INVALID)
#define ZASSERTX(x) if(x)return

#endif
