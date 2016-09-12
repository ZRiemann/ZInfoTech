/**@file type.c
 * @brief implement utility/type.h export functinos
 * @note
 * zhuweiping 2016-01-04 found
 */
#include "export.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>

int ZOK = ZEOK;// 0 // ZError OK
int ZFAIL = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
int ZMEM_INSUFFICIENT = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
int ZFUN_FAIL = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
int ZNOT_SUPPORT = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
int ZPARAM_INVALID = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
int ZAGAIN = ZEAGAIN;// (ZEMASK |6) // try again
int ZTIMEOUT = ZETIMEOUT;// (ZEMASK | 7) // operation time out
int ZNOT_EXIST = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
int ZNOT_INIT = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
int ZSTATUS_INVALID = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
int ZMEM_OUTOFBOUNDS = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
int ZCMD_STOP = ZECMD_STOP;// (ZEMASK | 12) // return stop command


int zversion(){
  return 0x010101;
}

const char* zsystem(){
  const char* msg = NULL;
#ifdef ZSYS_WINDOWS
  msg = "ZSYS_WINDOWS";
#else
#if defined(__SUNPRO_C)
  msg = "ZSYS_SUNPRO_C";
#elif (defined __GNUC__ && __GNUC__ >=4)
  msg = "ZSYS_GNUC_>=4";
#elif (defined __INTEL_COMPILER)
  msg = "ZSYS_INTEL_COMPILER";
#elif (defined __clang__)
  msg = "ZSYS__clang__";
#else
  msg = "ZSYS_LINUX";
#endif
#endif
  return msg;
}

const char* zhistory(){
  const char* msg = "V1.1.0 Z.Riemann founder\n";
  return msg;
}
