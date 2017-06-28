/**@file type.c
 * @brief implement utility/type.h export functinos
 * @note
 * zhuweiping 2016-01-04 found
 */
#include "export.h"
#include "auto_version.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <stdio.h>

const int ZOK = ZEOK;// 0 // ZError OK
const int ZFAIL = ZEFAIL;// (ZEMASK | 1) // ZError FAILED
const int ZMEM_INSUFFICIENT = ZEMEM_INSUFFICIENT;// (ZEMASK | 2) // memory insufficient
const int ZFUN_FAIL = ZEFUN_FAIL; // (ZEMASK | 3) // call function fail
const int ZNOT_SUPPORT = ZENOT_SUPPORT;// (ZEMASK | 4) // not support
const int ZPARAM_INVALID = ZEPARAM_INVALID; // (ZEMASK | 5) // parameter invalid
const int ZAGAIN = ZEAGAIN;// (ZEMASK |6) // try again
const int ZTIMEOUT = ZETIMEOUT;// (ZEMASK | 7) // operation time out
const int ZNOT_EXIST = ZENOT_EXIST; // (ZEMASK | 8) // Device/object ot exist
const int ZNOT_INIT = ZENOT_INIT;// (ZEMASK | 9)  // Device not init
const int ZSTATUS_INVALID = ZESTATUS_INVALID;// (ZEMASK | 10)  // device status invalid
const int ZMEM_OUTOFBOUNDS = ZEMEM_OUTOFBOUNDS;// (ZEMASK | 11) // memory out of bounds
const int ZCMD_STOP = ZECMD_STOP;// (ZEMASK | 12) // return stop command
const int ZCMP_EQUAL = ZECMP_EQUAL;// (ZEMASK | 13) // compare equal
const int ZCMP_GREAT = ZECMP_GREAT;// (ZEMASK | 14) // compare great
const int ZCMP_LITTLE = ZECMP_LITTLE;// (ZEMASK | 15) // compare little
const int ZCMD_RESTART = ZECMD_RESTART; // 16
const int ZEOF = ZE_EOF; //17
const int ZCAST_FAIL = ZECAST_FAIL;// (ZMASK | 18) // cast type fail
const int ZSOCK = ZESOCK_INVALID;// (ZEMASK | 19) // invalid socket

int zversion(){
    int version;
    version = major_version; version <<= 8;
    version |= minor_version; version <<= 8;
    version |= revision_version;
  return version;
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

const char* zhistory(char buf[1024]){
    sprintf(buf, "\n%s\n%s\n%s\n", version, build_date, git_rev);
    return build_date;
}
