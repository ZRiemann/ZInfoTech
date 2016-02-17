# ZInfoTech
ZRiemann Infomation Technology

zfun(...) support Windows and POSIX API

# ZInfoTech framework
zit/base
zit/thread
zit/thread/jet
  // only one jet per processor
  zjet_open();
  zjet_run();
  zjet_assign(ztsk_t* tsk)
  zjet_stop();
  ztet_close();
  zjet_assign(ztsk_t* tsk){
    
  }
zit/thread/mission
  #typedef struct zmission_t{
    zsem_t sem;
    zmutex_t mtx;
    zthr_t thr;
    zqueue_t in;
    zqueue_t out;
  }zmis_t;

  proc_mission{
    ztask_t* tsk;
    loop(tsk = pop(mission)){
      if(tsk->act)tsk->act(tsk->user, task->hint);
      if(tsk->release)tsk->release(task->user, task->hint);
    }
  }
zit/thread/task
  struct ztask_t;
  #typedef int (*ztask_free)(void* user, void* hint);
  #typedef int (*ztask_act)(void* user, void* hint);
  #typedef struct ztask_t{
    int priority; // 0-low(idel) 1-normal 2-(above normal)hight priority queue
    int level; // 0-sequence ; 1-immediate ; 2-normal
    void* user; // user data
    void* hint; // user hint
    ztask_act act; // action task
    ztask_free fre; // release user data
  }ztsk_t;
  
  ztask_pop(ztask_t** ztsk, void* user, ztask_free tf, ztask_act act); // default level2
  ztask_push();