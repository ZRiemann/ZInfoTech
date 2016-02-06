#ifndef _ZBASE_ERROR_H_
#define _ZBASE_ERROR_H_
/**@file zinc/base/error.h
 * @brief define ZInfoTech error numbers
 */

#define ZEMASK 0xff000000
#define ZEOK 0
#define ZEFAIL (ZEMASK | 1)
#define ZEMEM_INSUFFICIENT (ZEMASK | 2)
#define ZEFUN_FAIL (ZEMASK | 3)
#define ZENOT_SUPPORT (ZEMASK | 4)
#define ZEPARAM_INVALID (ZEMASK | 5)
#define ZEAGAIN (ZEMASK |6)
#define ZETIMEOUT (ZEMASK | 7)
#define ZENOT_EXIST (ZEMASK | 8)

#endif
