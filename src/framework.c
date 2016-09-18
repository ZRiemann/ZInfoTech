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
#include <stdlib.h>
#include <string.h>

int zobj_free(OPARG){
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
      //ret = zsem_init(&svr->sem_wait, 0);

      svr->dev.init = ztsk_svr_init;
      svr->dev.fini = ztsk_svr_fini;
      svr->dev.run = ztsk_svr_run;
      svr->dev.stop = ztsk_svr_stop;
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
  zcontainer_destroy(tsk_svr->observers, zobserver_destroy);
  zcontainer_destroy(tsk_svr->mis_wait, NULL);
  zcontainer_destroy(tsk_svr->tsk_recycle, zobj_free);
  zcontainer_destroy(tsk_svr->works, zobj_free);
  //zsem_uninit(&tsk_svr->sem_wait);
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
    *(((ztsk_t*)param[1].p)->observers) = (zcontainer_t)pair->value;
    *((int*)param[2].p) = ZOK;
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
  }
  return ZOK;
}

ZINLINE int zget_task(ztsk_svr_t *svr, ztsk_t **tsk){
  int ret;
  ret = zcontainer_pop(svr->tsk_recycle, (zvalue_t*)tsk);
  if(ZOK != ret){
    *tsk = (ztsk_t*)malloc(sizeof(ztsk_t));
    if(!*tsk){
      ret = ZMEM_INSUFFICIENT;
    }
    ret = ziatm_create(&(*tsk)->atm);
    if(ZOK != ret){
      free(*tsk);
      *tsk = NULL;
    }else{
      ziatm_xchg((*tsk)->atm, 1); // 
    }
  }
  return ret;
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
  zcontainer_foreach(svr->observers, foreach_task, &param);
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
    // first
    *dirty = ZTRUE;
    tsk->mission = (zvalue_t)mis;
    tsk->obj.operate = NULL;
    zcontainer_foreach(mis->operates, foreach_operate, (zvalue_t)tsk);
    zcontainer_push(mis->tasks, tsk);
    // !busy push mis_wait
    if(ZTRUE == ziatm_cas(mis->atm, ZFALSE, ZTRUE)){
      zcontainer_push(svr->mis_wait, mis);
      //zsem_post(sem_wait);
    }
  }else{
    // multi send
    ret = tsk->obj.clone(tsk, (zvalue_t*)&tsk_clone, NULL);
    if(ZOK == ret){
      tsk_clone->mission = (zvalue_t)mis;
      tsk_clone->obj.operate = NULL;
      zcontainer_foreach(mis->operates, foreach_operate, (zvalue_t)tsk_clone);
      zcontainer_push(mis->tasks, tsk_clone);
      // !busy push mis_wait
      if(ZTRUE == ziatm_cas(mis->atm, ZFALSE, ZTRUE)){
	zcontainer_push(svr->mis_wait, mis);
	//zsem_post(sem_wait);
      }
    }else{
      // ZERRC(ret);
    }
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
  zcontainer_foreach((*(tsk->observers)), foreach_post, (zvalue_t)param);
  if(ZNOT_EXIST == ret){
    ret = zrecycle_task(svr, tsk);
  }
  return ret;
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

  ZDBG("thread[%s] running...", thr->name);
  if(ZEOK != zthreadx_procbegin(thr)){
    zthreadx_procend(thr, ret);
    return 0;
  }
  while( ZTIMEOUT == zsem_wait(&(thr->exit), 0)){
    //if(ZOK != zsem_wait(&(svr->sem_wait), 200)){
    //  continue;
    //}
    ret = zcontainer_pop(svr->mis_wait, (zvalue_t)&mis);
    if(ZOK != ret){
      ZERRC(ret);
      continue;
    }

    // do tasks in mission, max 16 tasks
    cnt = 0;
    while(ZOK == zcontainer_pop(mis->tasks, (zvalue_t)&tsk)){
      tsk->obj.operate((zvalue_t)tsk, NULL, (zvalue_t)svr);
      //tsk->obj.release((zvalue_t)tsk, NULL, (zvalue_t)svr);
      zrecycle_task(svr, tsk);
      if(++cnt >= 16)break;
    }

    zcontainer_push(svr->mis_wait, mis);
    // set no task, last one pushed, for next mission
    //ziatm_cas(mis->atm, ZTRUE, ZFALSE);
  }
  zthreadx_procend(thr, ret);
  ZDBG("thread[%s] exit now.", thr->name);
  return 0;
}


static int ztsk_svr_init(OPARG){
  ztsk_svr_t *svr;

  svr = (ztsk_svr_t*)in;
  svr->worknum = *((int*)hint);
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
  
  while(zcontainer_pop(svr->works, (zvalue_t*)&thr)){
    zthreadx_cancel(thr);
    zthreadx_join(thr);
    free(thr);
  }
  
  return ZOK;
}
