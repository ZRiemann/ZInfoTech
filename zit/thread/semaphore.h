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

ZC_BEGIN

#define ZSEM_MAX 0xffff // specifies the maximum count of the semaphore.
#define ZNANO_SEC 1000000000 //999999999 // nanoseconds per second

ZAPI zerr_t zsem_init(zsem_t* sem, int value);
ZAPI zerr_t zsem_fini(zsem_t* sem);
ZAPI zerr_t zsem_post(zsem_t* sem);
ZAPI zerr_t zsem_wait(zsem_t* sem, int ms);// ZINFINIT|0-trywait|>0-timedwait
ZAPI zerr_t zsem_getvalue(zsem_t* sem, int* value);

ZC_END
/*
 * implements
 */
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
