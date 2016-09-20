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
#include <zit/framework/task.h>
ZC_BEGIN

ZAPI int zjet_init();
ZAPI int zjet_uninit();
ZAPI int zjet_run();
ZAPI int zjet_stop(int flag); // 0-act all assigned task; 1-drop all task; 2-backup task;

ZAPI int zjet_assign(ztsk_t* tsk);
ZAPI int zjet_getid(int* id); // Git mission id for sequence task.
ZAPI zstatus_t zjet_getstatus();

ZC_END

#endif
