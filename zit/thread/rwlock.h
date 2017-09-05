#ifndef _ZTHREAD_RWLOCK_H_
#define _ZTHREAD_RWLOCK_H_
/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#include <zit/base/platform.h>
#include <zit/base/error.h>

#ifdef ZSYS_POSIX
#include <pthread.h>
#include <errno.h>
typedef pthread_rwlock_t zrwlock_t;

#define zrwlock_init(rwlck) pthread_rwlock_init(rwlck, 0)
#define zrwlock_fini(rwlck) pthread_rwlock_destroy(rwlck)
#define zrwlock_rdlock(rwlck) pthread_rwlock_rdlock(rwlck)
#define zrwlock_tryrdlock(rwlck) (pthread_rwlock_tryrdlock(rwlck) ? ZEAGAIN : ZEOK)
#define zrwlock_wrlock(rwlck) pthread_rwlock_wrlock(rwlck)
#define zrwlock_trywrlock(rwlck) (pthread_rwlock_trywrlock(rwlck) ? ZEAGAIN : ZEOK)
#define zrwlock_timedrdlock(rwlck, sec, ret) do{   \
  struct timespec spec;\
  spec.tv_sec = sec;\
  spec.tv_nsec = 0;\
  ret = pthread_rwlock_timedrdlock(rwlck, &spec) ? ZEAGAIN : ZEOK;  \
}while(0)
#define zrwlock_timedwrlock(rwlck, sec, ret) do{\
    struct timespec spec;\
    spec.tv_sec = sec;\
    spec.tv_nsec = 900000000;\
    ret = pthread_rwlock_timedwrlock(rwlck, &spec) ? ZEAGAIN : ZEOK;\
}while(0)
#define zrwlock_unlock(rwlck, is_read) pthread_rwlock_unlock(rwlck)

#else
#include <windows.h>
typedef SRWLOCK zrwlock_t;

#define zrwlock_init(rwlck) InitializeSRWLock(rwlck)
#define zrwlock_fini(rwlck)
#define zrwlock_rdlock(rwlck) AcquireSRWLockShared(rwlck)
#define zrwlock_tryrdlock(rwlck) (TryAcquireSRWLockShared(rwlck) ? ZEOK : ZEAGAIN)
#define zrwlock_wrlock(rwlck) AcquireSRWLockExclusive(rwlck)
#define zrwlock_trywrlock(rwlck) (TryAcquireSRWLockExclusive(rwlck) ? ZEOK : ZEAGAIN)
#define zrwlock_timedrdlock(rwlck, sec, ret) do{\
CONDITION_VARIABLE cv;\
InitializeConditionVariable(&cv);\
ret = SleepConditionVariableSRW(&cv, rwlck, sec*1000, CONDITION_VARIABLE_LOCKMODE_SHARED)? ZEOK : ZEAGAIN;\
}while(0)
#define zrwlock_timedwrlock(rwlck, sec, ret) do{\
    CONDITION_VARIABLE cv;\
    InitializeConditionVariable(&cv);\
    ret = SleepConditionVariableSRW(&cv, rwlck, sec*1000, 0xffff)? ZEOK : ZEAGAIN;\
}while(0)
#define zrwlock_unlock(rwlck, is_read) (is_read ? ReleaseSRWLockShared(rwlck) : ReleaseSRWLockExclusive(rwlck))

#endif //ZSYS_POSIX

#endif
