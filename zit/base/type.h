/** @file zinc/basze/type.h
 *  @brief ZInfoTech types definitions
 *  @note zhuweiping 2016-01-04 found
 */
#ifndef _Z_TYPE_H_
#define _Z_TYPE_H_

#ifdef __cplusplus
extern "C" {
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
ZEXP int zversion();
ZEXP const char* zsystem();
ZEXP const char* zhistory();

#ifdef __cplusplus
}
#endif//__cplusplus

#endif//_Z_TYPE_H_
