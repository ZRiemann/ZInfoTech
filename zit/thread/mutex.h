#ifndef _ZTHREAD_MUTEX_H_
#define _ZTHREAD_MUTEX_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int zmutex_init(zmutex_t* mtx);
ZEXP int zmutex_uninit(zmutex_t* mtx);
ZEXP zmutex_t* zmutex_create();
ZEXP void zmutex_destroy(zmutex_t* mtx); // Destroy zmutex_create() 'mtx'. 
ZEXP int zmutex_lock(zmutex_t* mtx);
ZEXP int zmutex_unlock(zmutex_t* mtx);
ZEXP int zmutex_trylock(zmutex_t* mtx);

#ifdef ZSYS_POSIX
#define ZLOCK(x) pthread_mutex_lock(x)
#define ZUNLOCK(x) pthread_mutex_unlock(x)
#else
#define ZLOCK(x) EnterCriticalSection(x)
#define ZUNLOCK(x) LeaveCriticalSection(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
