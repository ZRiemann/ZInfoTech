#ifndef _ZBASE_PLATFORM_H_
#define _ZBASE_PLATFORM_H_

/* ZEXPORT = 0 use code directory
 * ZEXPORT = 1 export ZInfoTech API
 * ZEXPORT = 2 import ZInfoTech API
 */
#ifndef ZEXPORT
#define ZEXPORT 2
#endif//ZEXPORT

#if(defined(_WIN32) || defined(_WIN64))
#ifndef ZSYS_WINDOWS
#define ZSYS_WINDOWS
#endif
#define snprintf _snprintf
#else
#define ZSYS_POSIX
#endif

//#if(defined(_WIN32) || defined(_WIN64))
#if(defined(ZSYS_WINDOWS))
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
//#include <windows.h>
//#include <winsock2.h>
//#include <mswsock.h>
//#include <process.h>
//#include <ws2tcpip.h>
#pragma warning(disable:4996 4047 4022 4244)

#if (ZEXPORT == 0)
#define ZAPI
#elif (ZEXPORT == 1)
#define ZAPI __declspec(dllexport)
#elif (ZEXPORT == 2)
#define ZAPI __declspec(dllimport)
#endif
#define ZINLINE _inline

#ifndef inline
#define inline _inline
#endif

#elif(defined(ZSYS_POSIX))

#if defined(__SUNPRO_C)
#define ZAPI __global
#elif ((defined __GNUC__ && __GNUC__ >= 4) || defined __INTEL_COMPILER || defined __clang__)
#define ZAPI __attribute__ ((visibility("default")))
#else
#define ZAPI
#endif//__SUNPRO_C
#define ZINLINE inline
#endif // ZSYS_WINDOWS|ZSYS_POSIX

#ifdef __cplusplus
#define ZC_BEGIN extern "C" {
#define ZC_END }
#else
#define ZC_BEGIN
#define ZC_END
#endif //__cplusplus

#define ZUSE_INLINE 1 // control inline model or dll mode

#endif//_ZBASE_PLATFORM_H_
