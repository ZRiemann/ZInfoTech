#ifndef _ZTHREAD_JET_H_
#define _ZTHREAD_JET_H_

#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct zmission_t{
  zsem_t sem;
  zmutex_t mtx;
  zthr_t thr;
  zcontainer_t in;
  zcontainer_t out;
}zmis_t;

ZEXP int zjet_init();
ZEXP int zjet_uninit();
ZEXP int zjet_run();
ZEXP int zjet_stop(int flag); // 0-act all assigned task; 1-drop all task; 2-backup task
ZEXP int zjet_assign(ztsk_t* tsk);

#ifdef __cplusplus
}
#endif

#endif
