#ifndef _ZTHREAD_MUTEX_H_
#define _ZTHREAD_MUTEX_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int zmutex_init(zmutex_t* pmtx);
ZEXP int zmutex_uninit(zmutex_t* pmtx);
ZEXP int zmutex_lock(zmutex_t* pmtx);
ZEXP int zmutex_unlock(zmutex_t* pmtx);
ZEXP int zmutex_trylock(zmutex_t* pmtx);

#ifdef __cplusplus
}
#endif

#endif
