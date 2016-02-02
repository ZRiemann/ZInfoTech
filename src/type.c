/**@file type.c
 * @brief implement utility/type.h export functinos
 * @note
 * zhuweiping 2016-01-04 found
 */
#include "export.h"
#include <zit/base/type.h>

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
