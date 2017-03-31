/**@file zit/src/framework.c
 * @brief zit framework implements
 * @note
 * @email 25452483@qq.com
 * @history 2016-9-12 ZRiemann found
 */
#include "export.h"
#include <zit/framework/object.h>
#include <zit/framework/task.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/container.h>
#include <zit/thread/thread.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static zcontainer_t zg_timers;

int zobj_free(ZOP_ARG){
  free(in);
  return(ZOK);
}

int zobj_init(zobj_t *obj, zobj_t *parent, zobj_type_t type,\
	  zoperate op, zoperate cl, zoperate re, zoperate se){
  ZASSERT(!obj);
  obj->parent = parent;
  obj->type = type;
  obj->operate = op;
  obj->clone = cl;
  obj->release = re;
  obj->serialize = se;
  return(ZOK);
}


int zdev_init(zdev_t *dev,zobj_type_t type,  zoperate init, zoperate fini, zoperate run, zoperate stop){
  ZASSERT(!dev);
  zobj_initx((zobj_t*)dev, type, NULL);
  dev->owner = NULL;
  dev->mount = NULL;
  dev->status = ZUNINIT;
  dev->init = init;
  dev->fini = fini;
  dev->run = run;
  dev->stop = stop;
  return(ZOK);
}

//===============================
int zmis_init(OPARG){
  zmis_t *mis;
  zany_t *param;
  int ret;
  
  ret = ZOK;
  ZASSERT(!in);
  mis = (zmis_t*)in;
  param = (zany_t*)hint;
  if(param){
    zdev_init((zdev_t*)mis, param[0].i, (zoperate)param[1].p,(zoperate)param[2].p, \
	      (zoperate)param[3].p,(zoperate)param[4].p);
  }else{
    zdev_init((zdev_t*)mis, ZOBJ_TYPE_DUMMY, NULL,NULL,NULL,NULL);
  }
  if(ZOK !=(ret = zcontainer_create(&mis->tasks, /*ZCONTAINER_LIST*/ZCONTAINER_CKQUEUE))){
    return(ret);
  }
  if(ZOK != (ret = zcontainer_create(&mis->operates, ZCONTAINER_LIST))){
    zcontainer_destroy(mis->tasks, NULL);
    return(ret);
  }
  if(ZOK != (ret = ziatm_create(&mis->atm))){
    zcontainer_destroy(mis->tasks, NULL);
    zcontainer_destroy(mis->operates, NULL);
  }
  mis->mode = ZMIS_MODE_SERIAL;
  
  ZERRC(ret);
  return(ret);
}

int zmis_fini(OPARG){
  zmis_t *mis;
  mis = (zmis_t*)in;
  zcontainer_destroy(mis->tasks, zobj_free);
  zcontainer_destroy(mis->operates, zobj_free);
  ziatm_destroy(mis->atm);
  return(ZOK);
}

int zmis_attach_op(zmis_t *mis, zobj_type_t type, zoperate op){
  // just push, ignore repeat push; user avoid repeat
  zpair_t *pair;
  ZASSERT(!mis || !op);
  pair = (zpair_t*)malloc(sizeof(zpair_t));
  ZASSERTC(!pair, ZMEM_INSUFFICIENT);
  pair->key.i = type;
  pair->value = (zvalue_t)op;
  zcontainer_pushfront(mis->operates, (zvalue_t)pair);
  return(ZOK);
}
//===============================
// task implements
static int ztsk_svr_init(OPARG);
static int ztsk_svr_fini(OPARG);
static int ztsk_svr_run(OPARG);
static int ztsk_svr_stop(OPARG);

int ztsk_svr_create(ztsk_svr_t **tsk_svr){
  int ret;
  ztsk_svr_t *svr;
  do{
      ret = ZOK;
      ZASSERTBC(!tsk_svr, ZPARAM_INVALID);
      svr = *tsk_svr = (ztsk_svr_t*)malloc(sizeof(ztsk_svr_t));
      ZASSERTBC(!svr,ZMEM_INSUFFICIENT);
      memset(svr, 0, sizeof(ztsk_svr_t));
      zdev_init((zdev_t*)svr, ZOBJ_TYPE_DUMMY, ztsk_svr_init, ztsk_svr_fini, ztsk_svr_run, ztsk_svr_stop);
      ret = zcontainer_create(&svr->observers, ZCONTAINER_LIST);
      ZASSERTB(ZOK != ret);
      ret = zcontainer_create(&svr->mis_wait, ZCONTAINER_CKQUEUE);
      ZASSERTB(ZOK != ret);
      ret = zcontainer_create(&svr->tsk_recycle, ZCONTAINER_CKQUEUE);
      ZASSERTB(ZOK != ret);
      ret = zcontainer_create(&svr->works, ZCONTAINER_LIST);
      ZASSERTB(ZOK != ret);
      //ret = zcontainer_create(&svr->mis_pending, ZCONTAINER_LIST);
      //ZASSERTB(ZOK != ret);
      ret = zsem_init(&svr->sem_wait, 0);
      ZASSERTB(ZOK != ret);
      ret = ziatm_create(&svr->atm);
      svr->worknum = 4;
  }while(0);  
  if(ZOK != ret && svr){
    ztsk_svr_destroy(svr);
  }
  return ret;
}


static int zobserver_destroy(OPARG){
  zcontainer_destroy(((zpair_t*)in)->value, NULL); // free list<zmis_t*>
  return ZOK;
}

/*
static int zrecycle_destroy(OPARG){
  free(((zpair_t*)in)->value); // free task
  return ZOK;
}
*/
int ztsk_svr_destroy(ztsk_svr_t *tsk_svr){
  ZASSERT(!tsk_svr);
  zcontainer_destroy(tsk_svr->observers,  zobserver_destroy);
  zcontainer_destroy(tsk_svr->mis_wait, NULL);
  zcontainer_destroy(tsk_svr->tsk_recycle, zobj_free);
  zcontainer_destroy(tsk_svr->works, NULL);// sotp free zthr_t*
  //zcontainer_destroy(tsk_svr->mis_pending, NULL);
  ziatm_destroy(tsk_svr->atm);
  zsem_uninit(&tsk_svr->sem_wait);
  free(tsk_svr);
  return ZOK;
}

static int foreach_mis(OPARG){
  zpair_t *param = (zpair_t*)hint;
  //  zmis_t *mis = in;
  if(in == param->value){
    *((int*)param->key.p) = ZTRUE;
    return ZCMD_STOP;
  }
  return ZOK;
}
static int foreach_observer(OPARG){
  int ret;
  zpair_t *pair;
  zany_t *param;
  zcontainer_t zmis;
  int find;
  zpair_t hint1;
  
  ret = ZOK;
  pair= (zpair_t*)in;
  param = (zany_t*)hint;
  find = ZFALSE;
  hint1.key.p = (zptr_t)&find;
  hint1.value = param[1].p;
  if(pair->key.i == param[2].i){
    *((int*)param[0].p) = ZTRUE;
    ret = ZCMD_STOP;
    zmis = (zcontainer_t)pair->value;
    // push zcontainer_push(param[1].p);
    zcontainer_foreach(zmis, foreach_mis, (zvalue_t)&hint1);
    if(find == ZFALSE){
      zcontainer_push(zmis, param[1].p);
    }
  }
  return ret;
}

int ztsk_svr_observer(ztsk_svr_t *svr, zmis_t *mis, zobj_type_t task_type){
  int ret;
  int find;
  zpair_t* pair;
  zany_t hint[3];
  zcontainer_t cont;

  ZASSERT(!svr || !mis);
  find = ZFALSE;
  ret = ZOK;
  hint[0].p = (zptr_t)&find;
  hint[1].p = (zptr_t)mis;
  hint[2].i = task_type;
  zcontainer_foreach(svr->observers, foreach_observer, (zvalue_t)hint);
  if(ZFALSE == find){
    pair = (zpair_t*)malloc(sizeof(zpair_t));
    ZASSERTC(!pair, ZMEM_INSUFFICIENT);
    ret = zcontainer_create(&cont, ZCONTAINER_LIST);
    if(ZOK != ret){
      free(pair);
    }else{
      pair->key.i = task_type;
      pair->value = cont;
      zcontainer_push(cont, mis);
      zcontainer_push(svr->observers, (zvalue_t)pair);
    }
  }
  return ret;
}

static int foreach_erase(OPARG){
  zpair_t *pair = (zpair_t*)hint;
  zpair_t *pair1 = (zpair_t*)in;
  if(pair1->key.i == pair->key.i){
    zcontainer_erase(pair1->value, pair->value, NULL);
    return ZCMD_STOP;
  }
  return ZOK;
}

int ztsk_svr_unobserver(ztsk_svr_t *svr, zmis_t *mis, zobj_type_t task_type){
  zpair_t pair;

  pair.key.i = task_type;
  pair.value = mis;
  zcontainer_foreach(svr->observers, foreach_erase, (zvalue_t)&pair);
  return ZOK;
}

static int foreach_task(OPARG){
  zany_t *param = (zany_t*)hint;
  zpair_t *pair = (zpair_t*)in;
  
  if(pair->key.i == param[0].i){
    ((ztsk_t*)param[1].p)->observers = (zcontainer_t)pair->value;
    *((int*)param[2].p) = ZOK;
    return(ZCMD_STOP);
  }
  return ZOK;
}

ZAPI int ztsk_svr_unref(zvalue_t tsk, zvalue_t *out, zvalue_t ref){
  *((int*)ref) = ziatm_dec(((ztsk_t*)tsk)->atm);
  return ZOK;
}

ZINLINE int zrecycle_task(ztsk_svr_t *svr, ztsk_t *tsk){
  int hint = 0;
  if(tsk->obj.release){
    tsk->obj.release(tsk, NULL, (zvalue_t)&hint);
  }//else{
  ztsk_svr_unref(tsk, NULL, (zvalue_t)&hint);
    //}
  if(0 == hint){
    zcontainer_push(svr->tsk_recycle, tsk);
#if ZTRACE_FRAMEWORK
    ZDBG("recycle<size:%d> task<%p>", zcontainer_size(svr->tsk_recycle), tsk);
#endif
  }else{
#if ZTRACE_FRAMEWORK
    ZDBG("task reference<%d>, recycle another time.", hint);
#endif
  }
  return ZOK;
}

int ztsk_svr_recycle_task(ztsk_svr_t *svr, ztsk_t *tsk){
  return(zrecycle_task(svr, tsk));
}

ZINLINE int zget_task(ztsk_svr_t *svr, ztsk_t **tsk){
  int ret;
  ret = zcontainer_pop(svr->tsk_recycle, (zvalue_t*)tsk);
  if(ZOK != ret){
#if ZTRACE_FRAMEWORK
    ZDBG("task_buf<size>: %d, malloc memory for task...", zcontainer_size(svr->tsk_recycle));
#endif
    *tsk = (ztsk_t*)malloc(sizeof(ztsk_t));
    if(!*tsk){
      ret = ZMEM_INSUFFICIENT;
    }else{
      ret = ziatm_create(&(*tsk)->atm);
      if(ZOK != ret){
	free(*tsk);
	*tsk = NULL;
      }
    }
  }
  if(ZOK == ret){
    memset(&((*tsk)->obj), 0, sizeof(zobj_t));
    ziatm_xchg((*tsk)->atm, 1);
    (*tsk)->obj.clone = ztsk_clone_ref;
  }
#if ZTRACE_FRAMEWORK
  ZDBG("task_buf<size: %d>", zcontainer_size(svr->tsk_recycle));
#endif
  return ret;
}

int ztsk_clone_ref(ZOP_ARG){
  ztsk_t *tsk;
#if ZTRACE_FRAMEWORK
  zspin_t ref;
#endif
  tsk = (ztsk_t*)in;
#if ZTRACE_FRAMEWORK
  ref = ziatm_inc(tsk->atm);
#else
  ziatm_inc(tsk->atm);
#endif
  *out = in;
#if ZTRACE_FRAMEWORK
  ZDBG("tsk<ptr:%p, ref:%d", tsk, ref);
#endif
  return(ZOK);
}

int ztsk_clone_new(ZOP_ARG){
  //ztsk_svr_gettask();
  ztsk_t *tsk;
  ztsk_t *tsk_clone;
  ztsk_svr_t *svr;
  int ret;

  tsk = (ztsk_t*)in;
  svr = (ztsk_svr_t*)hint;
  ret = zget_task(svr, &tsk_clone);
  if(ZOK == ret){
    // copy context to tsk_clone
    memcpy(&tsk_clone->obj, &tsk->obj, sizeof(zobj_t));
    tsk_clone->mission = tsk->mission;
    tsk_clone->hint = tsk->hint;
    tsk_clone->param[0] = tsk->param[0];
    tsk_clone->param[1] = tsk->param[1];
    tsk_clone->param[2] = tsk->param[2];
    tsk_clone->observers = tsk->observers;
    *out = (zvalue_t)tsk_clone;
  }
  ZERRC(ret);
  return(ret);
}


int ztsk_svr_gettask(ztsk_svr_t *svr, ztsk_t **tsk, zobj_type_t task_type){
  int ret;
  zany_t param[3];

  ZASSERT(!svr || !tsk);
  ret = zget_task(svr, tsk); 
  ZASSERTR(ret);
  param[0].i = task_type;
  param[1].p = (zptr_t)*tsk;
  param[2].p = (zptr_t)&ret;
  ret = ZNOT_EXIST;
  (*tsk)->obj.type = task_type;
  zcontainer_foreach(svr->observers, foreach_task, (zvalue_t)param);
  if(ZOK != ret){
    zrecycle_task(svr, *tsk);
    *tsk = NULL;
  }
  return ret;
}

static int foreach_operate(OPARG){
  zpair_t *pair = in;
  ztsk_t *tsk = (ztsk_t*)hint;
  if(tsk->obj.type == pair->key.i){
    tsk->obj.operate = (zoperate)pair->value;
    return(ZCMD_STOP);
  }
  return ZOK;
}

static int foreach_post(OPARG){
  zany_t *param;
  zmis_t *mis;
  ztsk_t *tsk;
  ztsk_t *tsk_clone;
  int *dirty;
  int ret;
  ztsk_svr_t *svr;

  param = (zany_t*)hint;
  mis = (zmis_t*)in;
  tsk = (ztsk_t*)param[1].p;
  dirty = (int*)param[0].p;
  svr = (ztsk_svr_t*)param[2].p;
  if(ZNOT_EXIST == *dirty){
    *dirty = ZOK;
  }
  ret = tsk->obj.clone(tsk, (zvalue_t*)&tsk_clone, (zvalue_t)svr);
  if(ZOK == ret){
    tsk_clone->mission = (zvalue_t)mis;
    tsk_clone->obj.operate = NULL;
    zcontainer_foreach(mis->operates, foreach_operate, (zvalue_t)tsk_clone);
#if ZTRACE_FRAMEWORK
    ZDBG("push task<ptr:%p, ID:%04x> to mis<ptr:%p, type:%04x>", tsk_clone, tsk_clone->obj.type, mis, mis->dev.dev_type);
#endif
    ret = ziatm_lock(svr->atm);
    if(ZOK != ret){
      ZERRC(ret);
      return ret;
    }
    zcontainer_push(mis->tasks, tsk_clone);
    // !busy push mis_wait
    if(ZTRUE == ziatm_cas(mis->atm, ZFALSE, ZTRUE)){
#if ZTRACE_FRAMEWORK
      ZDBG("push mission to task wait.");
#endif
      zcontainer_push(svr->mis_wait, mis);
      zsem_post(&svr->sem_wait);
    }
    ziatm_unlock(svr->atm);
  }else{
    ZERRC(ret);
  }
  return ZOK;
}

int ztsk_svr_post(ztsk_svr_t *svr, ztsk_t *tsk){
  int ret;
  zany_t param[3];
  param[0].p = (zptr_t)&ret;
  param[1].p = (zptr_t)tsk;
  param[2].p = (zptr_t)svr;

  ret = ZNOT_EXIST;
  zcontainer_foreach(tsk->observers, foreach_post, (zvalue_t)param);
  ret = zrecycle_task(svr, tsk);
  return ret;
}

ZINLINE void post_mis(ztsk_svr_t* svr, zmis_t *mis){
  int ret;
  ret = ziatm_lock(svr->atm);
  if(ZOK != ret){
    ZERRC(ret);
    return;
  }
  if(0 < zcontainer_size(mis->tasks)){
    // push mis_wait again
    zcontainer_push(svr->mis_wait, mis);
    zsem_post(&svr->sem_wait);
#if ZTRACE_FRAMEWORK
    ZDBG("push to mis wait...");
#endif
  }else{
    // set no task, last one pushed, for next mission
    ret = ziatm_cas(mis->atm, ZTRUE, ZFALSE);
#if ZTRACE_FRAMEWORK
    ZDBG("mission has no task.");
#endif
  }
  ziatm_unlock(svr->atm);
}

static zthr_ret_t ZCALL zproc_tsk_svr(void* param){
  int ret;
  zthr_t* thr;
  ztsk_svr_t* svr;
  zmis_t *mis;
  ztsk_t *tsk;
  int cnt;
  
  thr = (zthr_t*)param;
  svr = (ztsk_svr_t*)thr->param; // for user parameter
  ret = ZEOK;
  cnt = 0;

  if(ZEOK != zthreadx_procbegin(thr)){
    zthreadx_procend(thr, ret);
    return 0;
  }

  while( ZTIMEOUT == zsem_wait(&(thr->exit), 0)){
    if(ZOK != zsem_wait(&(svr->sem_wait), 300)){
      continue;
    }
#if ZTRACE_FRAMEWORK
    else{
      ZDBG("wait sem ok");
    }
#endif

    ret = zcontainer_pop(svr->mis_wait, (zvalue_t)&mis);
    if(ZOK != ret){
      ZERRC(ret);
      continue;
    }
#if ZTRACE_FRAMEWORK
    else{
      ZDBG("get mis ok");
    }
#endif
    // do tasks in mission, max 16 tasks
    if(ZOK != zcontainer_pop(mis->tasks, (zvalue_t)&tsk)){
      zerr("not task in mission...");
      post_mis(svr,mis);
      continue;
    }
#if ZTRACE_FRAMEWORK
    else{
      ZDBG("get task ok");
    }
#endif
    
    if(ZMIS_MODE_CONCURRENT == mis->mode){
      // push mis to mis_wait for next thread do concurrent task
      post_mis(svr, mis);
      tsk->obj.operate((zvalue_t)tsk, (zvalue_t*)&thr, (zvalue_t)svr);
      zrecycle_task(svr, tsk);
#if ZTRACE_FRAMEWORK
      ZDBG("do concurrent task ok.");
#endif
    }else{      
      cnt = 1;
#if ZTRACE_FRAMEWORK
      ZDBG("begin do serial tasks...");
#endif
      tsk->obj.operate((zvalue_t)tsk, (zvalue_t*)&thr, (zvalue_t)svr);
      zrecycle_task(svr, tsk);
#if ZTRACE_FRAMEWORK
      ZDBG("do <%d> serial tasks ok.",cnt);
#endif
      while(ZOK == zcontainer_pop(mis->tasks, (zvalue_t)&tsk)){
	tsk->obj.operate((zvalue_t)tsk, (zvalue_t*)&thr, (zvalue_t)svr);
	zrecycle_task(svr, tsk);
	++cnt;
#if ZTRACE_FRAMEWORK
	ZDBG("do <%d> serial tasks ok.",cnt);
#endif
	if(cnt >= 16)break;
      }
      post_mis(svr, mis);
    }
  }
  zthreadx_procend(thr, ret);
  return 0;
}

static int ztsk_svr_init(OPARG){
  ztsk_svr_t *svr;

  svr = (ztsk_svr_t*)in;
  if(hint)svr->worknum = *((int*)hint);
  return ZOK;
}

static int ztsk_svr_fini(OPARG){
  return ZOK;
}

static int ztsk_svr_run(OPARG){
  int ret;
  ztsk_svr_t *svr;
  zthr_t *thr;
  int i;

  svr = (ztsk_svr_t*)in;
  ret = ZOK;
  
  for(i=0; i<svr->worknum; i++){
    thr = (zthr_t*)malloc(sizeof(zthr_t)); if(!thr)break;
    memset(thr, 0, sizeof(zthr_t));
    snprintf(thr->name, 64, "thr_task[%d]", i); // set task server thread name
    thr->param  = (zvalue_t)svr;
    ret = zthreadx_create(thr, zproc_tsk_svr);
    if(ZOK != ret){
      free(thr);
      break;
    }
    zcontainer_push(svr->works, (zvalue_t)thr);
  }
  svr->worknum = i;
  return ZOK;
}

static int ztsk_svr_stop(OPARG){
  ztsk_svr_t *svr;
  zthr_t *thr;

  svr = (ztsk_svr_t*)in;
  zthreadx_cancelall();
  while(ZOK == zcontainer_pop(svr->works, (zvalue_t*)&thr)){
    //    zthreadx_cancel(thr);
    zthreadx_join(thr);
    free(thr);
  }
  
  return ZOK;
}

int ztsk_timer_init(){
  if(zg_timers){return ZOK;}
  return zcontainer_create(&zg_timers, ZCONTAINER_LIST);
}

int ztsk_timer_fini(zoperate op_free){
  if(zg_timers){
    zcontainer_destroy(zg_timers, op_free);
  }
  return ZOK;
}

int ztsk_timer_add(ztsk_timer_t *timer){
  ZASSERT(!timer);
  return zcontainer_push(zg_timers, timer);
}

int ztsk_timer_update(ztsk_timer_t *timer, time_t tmstemp){
  timer->timestemp = tmstemp;
  return ZOK;
}

static int ztimer_trigger(ZOP_ARG){
  ztsk_t *tsk;
  ztsk_timer_t *timer;
  time_t now;
  int ret;

  time(&now);
  timer = (ztsk_timer_t*)in;
  ret = ZOK;

  if(!ZTSK_TIMER_ISENABLE(timer)){
    return ret;
  }

  if(now >= (timer->timestemp + timer->interval)){
    if(!ZTSK_TIMER_ISACT(timer)){
      ZWAR("timer<%x> blocked", timer->tsk_type);
      timer->timestemp = now;
      return ret;
    }
    ret = ztsk_svr_gettask((ztsk_svr_t*)hint, &tsk, ((ztsk_timer_t*)in)->tsk_type);
#if ZTRACE_FRAMEWORK
    ZERRCX(ret);
#endif
    ZASSERT(ret);
    timer->timestemp = now;
    ZTSK_TIMER_CLRACT(timer);
    tsk->hint = timer;
    ret = ztsk_svr_post((ztsk_svr_t*)hint, tsk);
#if ZTRACE_FRAMEWORK
    ZDBG("post timer<%x> %d", timer->tsk_type, ret);
#endif

  }
  return ret;
}

int ztsk_timer_trigger(ztsk_svr_t *svr){
  return zcontainer_foreach(zg_timers, ztimer_trigger, (zvalue_t)svr);
}

int ztsk_timer_set(ztsk_timer_t *timer, int tsk_type, int interval){
  timer->tsk_type = tsk_type;
  time(&timer->timestemp);
  timer->interval = interval;
  ZTSK_TIMER_SETENABLE(timer);
  ZTSK_TIMER_SETACT(timer);
  return ZOK;
}
