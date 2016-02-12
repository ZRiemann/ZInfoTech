#include "export.h"
#include <zit/thread/thread_def.h>
#include <zit/thread/thread.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>

// normal thread control
int zthread_create(zthr_id_t* id, zproc_thr proc, void* param){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_create(id, NULL, proc, param)))ret = errno;
#else//ZSYS_WINDOWS
  if( NULL == (*id = (zhr_id_t)_beginthreadex(NULL, 0, proc, param, 0, NULL)))ret = GetLastError();
#endif
  ZDBG("zthread_create[id:%p] %s", id, zstrerr(ret));
  return ret;

} 

int zthread_detach(zthr_id_t* id){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_detach(*id)))ret = errno;
#else//ZSYS_WINDOWS
  if(0 == CloseHandle(*id))ret = GetLastError();
#endif
  ZDBG("zthread_detach[id:%p] %s", id, zstrerr(ret));
  return ret;
}

int zthread_join(zthr_id_t* id){
  int ret = ZEOK;
  void* result;
  ZMSG("zthread_join(id:%p) begin...", id);
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_join(*id, &result)))ret = errno;
#else//ZSYS_WINDOWS
  ret = WaitForSingleObject(*id, ZINFINIT);
  switch(ret){
  case WAIT_OBJECT_0:
    ret = ZEOK; break;
  case WAIT_TIMEOUT:
    ret = ZETIMEDOUT;break;
  case WAIT_ABANDONED:
    ret = ZEFUN_FAIL;break;
  case WAIT_FAILED:
    ret = GetLastError();break;
  default:
    ret = ZEFUN_FAIL;break;
  }
#endif
  ZMSG("zthread_join(id:%p) end %s", id, zstrerr(ret));
  return ret;
}

int zthread_cancel(zthr_id_t* id){
  int ret = ZEOK;
  ZMSG("zthread_cancel(id:%p) begin...", id);
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_cancel(*id)))ret = errno;
#else//ZSYS_WINDOWS
  // dummy, just return ZEOK.
#endif
  ZMSG("zthread_cancel(id:%p) end %s", id, zstrerr(ret));
  return ret;
}

// threads manager
int zthreadx_create(zthr_attr_t* attr, zproc_thr proc){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_create(&(attr->id), NULL, proc, (void*)attr)))ret = errno;
#else//ZSYS_WINDOWS
  if( NULL == (attr->id = (zhr_id_t)_beginthreadex(NULL, 0, proc, (void*)attr, 0, NULL)))ret = GetLastError();
#endif
  ZDBG("zthreadx_create[name:%s] %s", attr->name, zstrerr(ret));
  return ret;
} 

int zthreadx_detach(zthr_attr_t* attr){
  int ret = ZEOK;
#ifdef ZSYS_POSIX
  if(0 != (ret = pthread_detach(attr->id)))ret = errno;
#else//ZSYS_WINDOWS
  if(0 == CloseHandle(attr->id))ret = GetLastError();
#endif
  ZDBG("zthread_detach[name:%s] %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_join(zthr_attr_t* attr){
  int ret = ZEOK;
  void* result;
  ZMSG("zthread_join(name:%s) begin...", attr->name);
#ifdef ZSYS_POSIX
  if( 0 != (ret = pthread_join(attr->id, &result)))ret = errno;
#else//ZSYS_WINDOWS
  ret = WaitForSingleObject(attr->id, ZINFINIT);
  switch(ret){
  case WAIT_OBJECT_0:
    ret = ZEOK; break;
  case WAIT_TIMEOUT:
    ret = ZETIMEDOUT;break;
  case WAIT_ABANDONED:
    ret = ZEFUN_FAIL;break;
  case WAIT_FAILED:
    ret = GetLastError();break;
  default:
    ret = ZEFUN_FAIL;break;
  }
#endif
  ZMSG("zthread_join(name:%s) end %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_cancel(zthr_attr_t* attr){
  int ret = ZEOK;
  ZMSG("zthread_cancel(name:%s) begin...", attr->name);
  zsem_post(&(attr->exit));
  ZMSG("zthread_cancel(name:%s) end %s", attr->name, zstrerr(ret));
  return ret;
}

int zthreadx_cancelall(){
  return ZENOT_SUPPORT;
}
int zthreadx_joinall(){
  return ZENOT_SUPPORT;
}
