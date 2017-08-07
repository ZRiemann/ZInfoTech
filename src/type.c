/**@file type.c
 * @brief implement utility/type.h export functinos
 * @note
 * zhuweiping 2016-01-04 found
 */
#include "export.h"
#include "auto_version.h"
#include <stdio.h>

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
