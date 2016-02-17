#include "export.h"
#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/thread/thread.h>
#include <zit/thread/jet.h>

ZEXP int zjet_init(){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}

ZEXP int zjet_uninit(){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}

ZEXP int zjet_run(){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}

ZEXP int zjet_stop(int flag){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}

ZEXP int zjet_assign(ztsk_t* tsk){
  int ret = ZENOT_SUPPORT;
  ZERRC(ret);
  return ret;
}
