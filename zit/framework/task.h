#ifndef _ZFR_TASK_H_
#define _ZFR_TASK_H_
/**@file zit/framework/task.h
 * @brief zit task definitions
 * @hote
 * @email 25452483@qq.com
 * @history
 * 2016-9-12 ZRiemann found
 */
#include <zit/framework/object.h>
#include <zit/base/queue.h>

 
typedef struct task_observer_s{
  int type; // observer type
  //  zlist_t observers; //
  zque_t observers;
}ztsk_obs_t;

// general task definition 
typedef struct ztask_s{
  zobj_t obj;
  zvalue_t user;
  zvalue_t param;
}ztsk_t;

ZAPI int ztsk_post(ztsk_t *tsk);
ZAPI int ztsk_send(ztsk_t *tsk); // must status independent

typedef struct zmission_s{
  zdev_t dev;
  uint16_t workers;
  zque_t *tasks;
  zoperate push;
  zoperate pop;
}zmis_t;

ZAPI int zmis_create(zmis_t **mis, int num, zque_t *tasks);
ZAPI int zmis_push(OPARG);
ZAPI int zmis_POP(OPARG);

typedef struct ztsk_server_s{
  zdev_t dev;
  zque_t observers;
}ztsk_svr_t;

ZAPI int ztsk_svr_create(ztsk_svr_t **tsk_svr);
// ztsk_svr_destroy(ztsk_svr_t *tsk_svr){tsk_svr->release(OPIN(tsk_svr));}
ZAPI int ztsk_svr_observer(ztsk_svr_t *svr, zmis_t *mis, zobj_type_t task_type);
// unobserver
ZAPI int ztsk_svr_gettask(ztsk_svr_t *svr, zobj_t **obj, zobj_type_t task_type);
/*
  init()/fini()/run()/stop() default implement
*/

#endif
