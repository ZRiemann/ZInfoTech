#ifndef _ZUTILITY_TRACECONSOLE_H_
#define _ZUTILITY_TRACECONSOLE_H_

#include <zit/base/platform.h>

ZC_BEGIN

ZAPI int ztrace_console(int level, void* user, const char* msg);

ZC_END

#endif
