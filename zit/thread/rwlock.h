#ifndef _ZTHREAD_RWLOCK_H_
#define _ZTHREAD_RWLOCK_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

ZC_BEGIN

ZAPI int zrwlock_init(zrwlock_t* mtx);
ZAPI int zrwlock_fini(zrwlock_t* mtx);
//ZAPI zrwlock_t* zrwlock_create();
//ZAPI void zrwlock_destroy(zrwlock_t* mtx); // Destroy zrwlock_create() 'mtx'. 
ZAPI int zrwlock_rdlock(zrwlock_t* mtx);
ZAPI int zrwlock_tryrdlock(zrwlock_t* mtx);
ZAPI int zrwlock_wrlock(zrwlock_t* mtx);
ZAPI int zrwlock_trywrlock(zrwlock_t* mtx);
ZAPI int zrwlock_timedrdlock(zrwlock_t* mtx, int sec);
ZAPI int zrwlock_timedwrlock(zrwlock_t* mtx, int sec);
ZAPI int zrwlock_unlock(zrwlock_t* mtx);

ZC_END

#endif
