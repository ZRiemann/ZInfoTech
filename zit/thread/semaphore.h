#ifndef _ZTHREAD_SEMAPHORE_H_
#define _ZTHREAD_SEMAPHORE_H_

#include <zit/base/platform.h>

#ifdef ZSYS_WINDOWS
#include <windows.h>
typedef HANDLE zsem_t;

#else
#include <semaphore.h>
typedef sem_t zsem_t;
#endif

#define ZSEM_MAX 0xffff // specifies the maximum count of the semaphore.
#define ZNANO_SEC 999999999 // nanoseconds per second

int zsem_init(zsem_t* sem, int value);
int zsem_fini(zsem_t* sem);
int zsem_post(zsem_t* sem);
int zsem_wait(zsem_t* sem, int ms);// ZINFINIT|0-trywait|>0-timedwait
int zsem_getvalue(zsem_t* sem, int* value);

/*
 * implements
 */
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <time.h>
#include <stdlib.h>

inline int zsem_init(zsem_t* sem, int value){
    int ret = ZEOK;
#ifdef ZSYS_POSIX
    if(0 != (ret = sem_init(sem, 0, value))){
        ret = errno;
    }
#else//ZSYS_WINDOWS
    *sem = CreateSemaphoreA(NULL, value, ZSEM_MAX, NULL);
    if (NULL == *sem) {
        ret = GetLastError();
    }
#endif
    return ret;
}

inline int zsem_fini(zsem_t* sem){
    int ret = ZEOK;
#ifdef ZSYS_POSIX
    if(0 != sem_destroy(sem)){
        ret = errno;
    }
#else//ZSYS_WINDOWS
    if(0 == CloseHandle(*sem)){
        ret = GetLastError();
    }
#endif
    return ret;
}

inline int zsem_post(zsem_t* sem){
    int ret = ZEOK;
#ifdef ZSYS_POSIX
    if(0 != sem_post(sem)){
        ret = errno;
    }
#else//ZSYS_WINDOWS
    if(0 == ReleaseSemaphore(*sem, 1, NULL)){
        ret = GetLastError();
    }
#endif
    return ret;
}

#ifdef ZSYS_WINDOWS
int zobj_wait(HANDLE h, int ms);
inline int zobj_wait(HANDLE h, int ms){
    int ret = WaitForSingleObject(h, ms);
    switch(ret){
    case WAIT_OBJECT_0:ret = ZEOK;break;
    case WAIT_TIMEOUT:ret = ZETIMEOUT;break;
    case WAIT_ABANDONED:ret =ZEFUN_FAIL;break;
    case WAIT_FAILED:ret = GetLastError();break;
    default:ret = ZEFUN_FAIL;break;
    }
    return ret;
}
#endif

inline int zsem_wait(zsem_t* sem, int ms){
    int ret = ZEOK;
    //ZDBG("sem_wait begin...");
#ifdef ZSYS_POSIX
    if(0 == ms){// trywait
        while((-1 == (ret =sem_trywait(sem))) && (errno == EINTR))continue;
        if((-1 == ret) && (EAGAIN == (ret = errno))){
            ret = ZETIMEOUT;
        }
    }else if( 0 < ms){// timed wait
        long ns;
        long sec;
        struct timespec ts;
        sec = ms/1000;
        ns = (ms%1000)*1000000;
        if( -1 == clock_gettime(CLOCK_REALTIME, &ts)){
            ret = errno;
            ZERRC(ret);
            return ret;
        }
        ts.tv_sec += sec;
        ts.tv_nsec += ns;
        ts.tv_sec += (ts.tv_nsec/ZNANO_SEC);
        ts.tv_nsec %=ZNANO_SEC;
        while((-1 == (ret = sem_timedwait(sem,&ts))) && (EINTR == errno))continue;
        if((-1 == ret) && (ETIMEDOUT == (ret = errno))){
            ret = ZETIMEOUT;
        }
    }else{// infinit wait
        //while((-1 == (ret =sem_wait(sem))) && ((ret = errno) == EINTR))continue;
        ret = sem_wait(sem);
        if(-1 == ret ){
            ret = errno;
        }
    }

#else//ZSYS_WINDOWS
    ret = zobj_wait(*sem, ms);
#endif
    return ret;
}

inline int zsem_getvalue(zsem_t* sem, int* value){
    int ret = ZEOK;
#ifdef ZSYS_POSIX
    if(0 != sem_getvalue(sem, value)){
        ret = errno;
    }
#else // ZSYS_WINDOWS
    ret = zobj_wait(*sem, 0);
    if(ZEOK == ret){
        if(0 == ReleaseSemaphore(*sem, 1, value)){
            ret = GetLastError();
        }
        (*value)++;
    }else{
        *value = 0;
    }
#endif
    return ret;
}

/**@fn int zsem_init(zsem_t* sem, int value)
   @brief initialize semaphore, for local processor
   @param zsem_t* sem [in] semaphore pointer
   @param int value [in] initial value for the semaphore
   @return ZEOK/[errno|lasterror]
*/
/**@fn int zsem_uninit(zsem_t* sem)
   @brief destroy an unnamed semaphore
   @param zsem_t* sem [in] 
   @return ZEOK/[errno|lasterror]
*/
/**@fn int zsem_post(zsem_t* sem)
   @brief unlock semaphore pointed by <sem>
   @param zsem_t* sem [in] semaphore pointer
   @return ZEOK/[errno|lasterror]
*/
/**@fn int zsem_wait(zsem_t* sem, int ms)
   @brief wait a sempahore by ms
   @param zsem_t* sem [in] semaphore pointer
   @param int ms [in] wait <ms> microsecond
   @return ZEOK|ZETIMEOUT|[errno|lasterror]
*/
#endif
