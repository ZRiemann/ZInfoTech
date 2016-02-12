#ifndef _ZTHREAD_THREAD_H_
#define _ZTHREAD_THREAD_H_

#include <zit/base/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int zthread_create(zthr_id_t* id, zproc_thr proc, void* param); 
ZEXP int zthread_detach(zthr_id_t* id);
ZEXP int zthread_join(zthr_id_t* id);
ZEXP int zthread_cancel(zthr_id_t* id);
  
// threads manager
ZEXP int zthreadx_create(zthr_attr_t* attr, zproc_thr proc);
ZEXP int zthreadx_detach(zthr_attr_t* attr);
ZEXP int zthreadx_cancel(zthr_attr_t* attr);
ZEXP int zthreadx_join(zthr_attr_t* attr);
ZEXP int zthreadx_wait(zthr_attr_t* attr, int ms);
ZEXP int zthreadx_cancelall(); // mast enable zthreadx_wait()
ZEXP int zthreadx_joinall();

#ifdef __cplusplus
}
#endif

#endif
