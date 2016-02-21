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
#define ZENOT_EXIST (ZEMASK | 8) // Device/object ot exist
#define ZENOT_INIT (ZEMASK | 9)  // Device not init
#define ZESTATUS_INVALID (ZEMASK | 10)  // device status invalid

#endif
