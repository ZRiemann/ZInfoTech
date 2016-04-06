#ifndef _ZTHREAD_MUTEX_H_
#define _ZTHREAD_MUTEX_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

ZC_BEGIN

ZAPI int zmutex_init(zmutex_t* mtx);
ZAPI int zmutex_uninit(zmutex_t* mtx);
ZAPI zmutex_t* zmutex_create();
ZAPI void zmutex_destroy(zmutex_t* mtx); // Destroy zmutex_create() 'mtx'. 
ZAPI int zmutex_lock(zmutex_t* mtx);
ZAPI int zmutex_unlock(zmutex_t* mtx);
ZAPI int zmutex_trylock(zmutex_t* mtx);

#ifdef ZSYS_POSIX
#define ZLOCK(x) pthread_mutex_lock(x)
#define ZUNLOCK(x) pthread_mutex_unlock(x)
#else
#define ZLOCK(x) EnterCriticalSection(x)
#define ZUNLOCK(x) LeaveCriticalSection(x)
#endif

ZC_END

#endif
