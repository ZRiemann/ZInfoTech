#ifndef _ZTHREAD_MUTEX_H_
#define _ZTHREAD_MUTEX_H_

#include <zit/base/platform.h>

#ifdef ZSYS_WINDOWS
#include <Windows.h>
typedef CRITICAL_SECTION zmtx_t;

#define zmtx_init(mtx) (InitializeCriticalSectionEx(mtx, 8192, CRITICAL_SECTION_NO_DEBUG_INFO) ? ZEOK : ZEFUN_FAIL)
#define zmtx_fini(mtx) DeleteCriticalSection(mtx)
#define zmtx_lock(mtx) EnterCriticalSection(mtx)
#define zmtx_unlock(mtx) LeaveCriticalSection(mtx)
#define zmtx_trylock(mtx) (TryEnterCriticalSection(mtx) ? ZEOK : ZEFUN_FAIL)

#else // ZSYS_WINDOWS

#include <pthread.h>
typedef pthread_mutex_t zmtx_t;

#define zmtx_init(mtx) (pthread_mutex_init(mtx, 0) ? ZEFUN_FAIL : ZEOK)
#define zmtx_fini(mtx) (pthread_mutex_destroy(mtx) ? ZEFUN_FAIL : ZEOK)
#define zmtx_lock(mtx) pthread_mutex_lock(mtx)
#define zmtx_unlock(mtx) pthread_mutex_unlock(mtx)
#define zmtx_trylock(mtx) (pthread_mutex_trylock(mtx) ? ZEFUN_FAIL : ZEOK)

#endif //ZSYS_WINDOWS

#endif
