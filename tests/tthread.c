#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include "tthread.h"

void ztst_thread(){
  ztst_semaphore();
}

void ztst_semaphore(){
  zsem_t sem;
  zsem_t sem1;
  ZMSG("test normal...");
  zsem_init(&sem,0);
  zsem_post(&sem);
  zsem_wait(&sem,100);
  zsem_uninit(&sem);
  
  ZMSG("test sem destroyed...");
  zsem_post(&sem1);
  zsem_wait(&sem1,100);
}
