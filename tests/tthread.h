#ifndef _ZTST_THREAD_H_
#define _ZTST_THREAD_H_

#include <zit/base/platform.h>

ZC_BEGIN

void ztst_thread();

void ztst_semaphore();
void ztst_mutex();
void ztst_thrctl();
void ztst_thrxctl(); // testing zthreadx_*()

ZC_END

#endif
