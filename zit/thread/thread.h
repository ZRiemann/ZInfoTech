#ifndef _ZTHREAD_THREAD_H_
#define _ZTHREAD_THREAD_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

ZC_BEGIN

ZAPI int zthread_create(zthr_id_t* id, zproc_thr proc, void* param); 
ZAPI int zthread_detach(zthr_id_t* id);
ZAPI int zthread_join(zthr_id_t* id);
ZAPI int zthread_cancel(zthr_id_t* id);
  
// threads manager
ZAPI int zthreadx_create(zthr_t* thr, zproc_thr proc);
ZAPI int zthreadx_detach(zthr_t* thr);
ZAPI int zthreadx_cancel(zthr_t* thr);
ZAPI int zthreadx_join(zthr_t* thr);
ZAPI int zthreadx_cancelall();
ZAPI int zthreadx_joinall(); // only call in main thread
ZAPI int zthreadx_procbegin(zthr_t* thr);
ZAPI int zthreadx_procend(zthr_t* thr, int result);

ZC_END

#endif
