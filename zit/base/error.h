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
#define ZE_END (12)

#define ZASSERT(x) if(x){return(ZEPARAM_INVALID);}
#define ZASSERTX(x) if(x){return;}

#endif
