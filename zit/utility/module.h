#ifndef _ZUTILITY_MODULE_H_
#define _ZUTILITY_MODULE_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>

#ifdef __cplusplus
exter "C" {
#endif

ZEXP int zmodule_name(char* path, char* name);

#ifdef __cplusplus
}
#endif

/**@fn int zmodule_name(char* path, char* name)
   @brief get the executable module path and name
   @param char* path [out] buffer for path[256]
   @param char* name [out] buffer for name[64]
   @return ZEOK|ZENOT_EXIST|[errno|lasterror]
   @note if path set NULL, ignore path, so seem to name.
 */

#endif
