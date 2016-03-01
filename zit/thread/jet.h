#ifndef _ZTHREAD_JET_H_
#define _ZTHREAD_JET_H_
/**@file zit/thread/jet.h
   @brief task engine
   @note
   author    date       history
   ZRiemann  2016-02-19 found, one porcessor only one jet!
   ZRiemann  2016-02-23 fix dead lock/loop bug. 
 */
#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif

ZEXP int zjet_init();
ZEXP int zjet_uninit();
ZEXP int zjet_run();
ZEXP int zjet_stop(int flag); // 0-act all assigned task; 1-drop all task; 2-backup task;

ZEXP int zjet_assign(ztsk_t* tsk);
ZEXP int zjet_getid(int* id); // Git mission id for sequence task.
ZEXP zstatus_t zjet_getstatus();
#ifdef __cplusplus
}
#endif

#endif
