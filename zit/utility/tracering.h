#ifndef _ZUTILITY_TRACE_RING_H_
#define _ZUTILITY_TRACE_RING_H_

#include <zit/base/platform.h>
#include <zit/base/trace.h>

ZC_BEGIN

ZAPI int ztracering_init(ztrace trace, void* user);
ZAPI int ztracering_uninit();
  
ZC_END

/**@file zit/utility/tracering.h
   @brief trace in back ground by jet and ringbuf
*/
#endif
