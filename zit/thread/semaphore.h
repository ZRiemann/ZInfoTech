#ifndef _ZTHREAD_SEMAPHORE_H_
#define _ZTHREAD_SEMAPHORE_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int zsem_init(zsem_t* sem, int value);
ZEXP int zsem_uninit(zsem_t* sem);
ZEXP zsem_t* zsem_create(int value);
ZEXP void zsem_destroy(zsem_t* sem); // Destroy zsem_create() 'sem'
ZEXP int zsem_post(zsem_t* sem);
ZEXP int zsem_wait(zsem_t* sem, int ms);// ZINFINIT|0-trywait|>0-timedwait

#ifdef __cplusplus
}
#endif

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
