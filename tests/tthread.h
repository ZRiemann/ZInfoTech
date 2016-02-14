#ifndef _ZTST_THREAD_H_
#define _ZTST_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

void ztst_thread();

void ztst_semaphore();
void ztst_mutex();
void ztst_thrctl();
void ztst_thrxctl(); // testing zthreadx_*()

#ifdef __cplusplus
}
#endif

#endif
