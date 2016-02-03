/**@file zutility/platform.h
 * @brief define ZInfoTech platform
 */
#ifndef _ZBASE_PLATFORM_H_
#define _ZBASE_PLATFORM_H_

#ifndef ZEXPORT
#define ZEXPORT 0
#endif
/* ZEXPORT = 0 use code directory
 * ZEXPORT = 1 export ZInfoTech API
 * ZEXPORT = 2 import ZInfoTech API
 */
#ifndef ZEXPORT
#define ZEXPORT 2
#endif//ZEXPORT

#if(defined(_WIN32) || defined(_WIN64))

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//#include <windows.h>
//#include <winsock2.h>
//#include <mswsock.h>
//#include <process.h>
//#include <ws2tcpip.h>

#define ZSYS_WINDOWS

#if (ZEXPORT == 0)
#define ZEXP
#elif (ZEXPORT == 1)
#define ZEXP __declspec(dllexport)
#elif (ZEXPORT == 2)
#define ZEXP __declspec(dllimport)
#endif

#else

#define ZSYS_POSIX

#if defined(__SUNPRO_C)
#define ZEXP __global
#elif ((defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER || defined __clang__)
#define ZEXP __attribute__ ((visibility("default")))
#else
#define ZEXP
#endif//__SUNPRO_C

#endif

#endif//_ZBASE_PLATFORM_H_
