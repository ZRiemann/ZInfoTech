#ifndef _ZUTILITY_TRACE_RING_H_
#define _ZUTILITY_TRACE_RING_H_

#include <zit/base/platform.h>
#include <zit/base/trace.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int ztracering_init(ztrace trace, void* user);
ZEXP int ztracering_uninit();
  
#ifdef __cplusplus
}
#endif
/**@file zit/utility/tracering.h
   @brief trace in back ground by jet and ringbuf
*/
#endif
