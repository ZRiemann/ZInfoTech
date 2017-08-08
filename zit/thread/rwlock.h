#ifndef _ZTHREAD_RWLOCK_H_
#define _ZTHREAD_RWLOCK_H_

#include <zit/base/platform.h>

#ifdef ZSYS_POSIX
#include <pthread.h>
#include <errno.h>
typedef pthread_rwlock_t zrwlock_t;
#else
#include <windows.h>
typedef SRWLOCK zrwlock_t;
#endif //ZSYS_POSIX

int zrwlock_init(zrwlock_t* rwlck);
int zrwlock_fini(zrwlock_t* rwlck);
// zrwlock_t* zrwlock_create();
// void zrwlock_destroy(zrwlock_t* rwlck); // Destroy zrwlock_create() 'rwlck'. 
int zrwlock_rdlock(zrwlock_t* rwlck);
int zrwlock_tryrdlock(zrwlock_t* rwlck);
int zrwlock_wrlock(zrwlock_t* rwlck);
int zrwlock_trywrlock(zrwlock_t* rwlck);
int zrwlock_timedrdlock(zrwlock_t* rwlck, int sec);
int zrwlock_timedwrlock(zrwlock_t* rwlck, int sec);
int zrwlock_unlock(zrwlock_t* rwlck, int is_read);

#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <stdlib.h>

zinline int zrwlock_init(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_init(rwlck, 0);
  if( 0 != ret )
  {
    ret = errno;
  }
#else//ZSYS_WINDOWS
  InitializeSRWLock(rwlck);
#endif
  return ret;
}

zinline int zrwlock_fini(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_destroy(rwlck);
  if( 0 != ret ){
    ret = ZEFUN_FAIL;
  }
#else//ZSYS_WINDOWS
  // do nothing
#endif
  return ret;
}

zinline int zrwlock_rdlock(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_rdlock(rwlck);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  AcquireSRWLockShared(rwlck);
#endif
  return ret;
}

zinline int zrwlock_tryrdlock(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_tryrdlock(rwlck);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = TryAcquireSRWLockShared(rwlck) ? ZOK : ZEAGAIN;
#endif
  return ret;
}

zinline int zrwlock_wrlock(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_wrlock(rwlck);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  AcquireSRWLockExclusive(rwlck);
#endif
  return ret;
}

zinline int zrwlock_trywrlock(zrwlock_t* rwlck){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_trywrlock(rwlck);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  ret = TryAcquireSRWLockExclusive(rwlck) ? ZOK : ZEAGAIN;
#endif
  return ret;
}

zinline int zrwlock_timedrdlock(zrwlock_t* rwlck, int sec){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  struct timespec spec;
  spec.tv_sec = sec;
  spec.tv_nsec = 0;
  ret = pthread_rwlock_timedrdlock(rwlck, &spec);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  CONDITION_VARIABLE cv;
  InitializeConditionVariable(&cv);
  ret = SleepConditionVariableSRW(&cv, rwlck, sec*1000, CONDITION_VARIABLE_LOCKMODE_SHARED)? ZEOK : ZEAGAIN;
#endif
  return ret;
}

zinline int zrwlock_timedwrlock(zrwlock_t* rwlck, int sec){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  struct timespec spec;
  spec.tv_sec = sec;
  spec.tv_nsec = 900000000;

  ret = pthread_rwlock_timedwrlock(rwlck, &spec);
  if(0 != ret){
    ret = ZEAGAIN;
  }
#else//ZSYS_WINDOWS
  CONDITION_VARIABLE cv;
  InitializeConditionVariable(&cv);
  ret = SleepConditionVariableSRW(&cv, rwlck, sec*1000, 0xffff)? ZEOK : ZEAGAIN;
#endif
  return ret;
}

zinline int zrwlock_unlock(zrwlock_t* rwlck, int is_read){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  ret = pthread_rwlock_unlock(rwlck);// if(0 != ret) ret = ZEFUN_FAIL;
#else//ZSYS_WINDOWS
  is_read ? ReleaseSRWLockShared(rwlck) : ReleaseSRWLockExclusive(rwlck);
#endif
  return ret;
}

#endif
