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
#include <zit/base/type.h>
#include <zit/thread/semaphore.h>
#include <zit/base/atomic.h>
#include <zit/thread/thread.h>

// general task definition 
typedef struct ztask_s{
  zobj_t obj;
  zvalue_t mission;
  zvalue_t hint;
  zany_t param[3];
  zcontainer_t observers; ///< list<mis_t*>*
  zatm_t atm; /// reference
}ztsk_t;
ZAPI int ztsk_clone_ref(ZOP_ARG); // inc reference for clone, default imp
ZAPI int ztsk_clone_new(ZOP_ARG); // get new task.
//================================
#define ZMIS_MODE_SERIAL 0
#define ZMIS_MODE_CONCURRENT 1

typedef struct zmission_s{
  zdev_t dev;
  int mode;              // 0- serial mode 1- concurrent mode
  zcontainer_t tasks;    // queue<ztsk_t*>
  zcontainer_t operates; // list< zpair_t< zobj_type_t, zoperate >* >
  zatm_t atm;             // ZFALSE - no task; ZTRUE - wait task
}zmis_t;

ZAPI int zmis_init(ZOP_ARG);
ZAPI int zmis_fini(ZOP_ARG);
ZAPI int zmis_attach_op(zmis_t *mis, zobj_type_t type, zoperate op);

typedef struct ztsk_server_s{
  zdev_t dev;
  zcontainer_t observers;// list< zpair_t< zobj_type_t, list<zmis_t*> >* > observer()/gettask()
  zcontainer_t mis_wait; // queue<mis_t*>
  //zcontainer_t mis_pending; // queue<mis_t*> avoid zombie task
  zcontainer_t tsk_recycle; // tsk buffer
  zcontainer_t works;    // list<zthr_t*>
  int worknum;           // work numbers
  zsem_t sem_wait;
  zatm_t atm;            // lock
}ztsk_svr_t;

ZAPI int ztsk_svr_create(ztsk_svr_t **tsk_svr);
ZAPI int ztsk_svr_destroy(ztsk_svr_t *tsk_svr);//{tsk_svr->release(OPIN(tsk_svr));}
ZAPI int ztsk_svr_observer(ztsk_svr_t *svr, zmis_t *mis, zobj_type_t task_type);
ZAPI int ztsk_svr_unobserver(ztsk_svr_t *svr, zmis_t *mis, zobj_type_t task_type);
ZAPI int ztsk_svr_gettask(ztsk_svr_t *svr, ztsk_t **tsk, zobj_type_t task_type);
ZAPI int ztsk_svr_post(ztsk_svr_t * svr, ztsk_t *tsk);
ZAPI int ztsk_svr_unref(zvalue_t tsk, zvalue_t *out, zvalue_t ref);
ZAPI int ztsk_svr_recycle_task(ztsk_svr_t *svr, ztsk_t *tsk);
/*
  init()/fini()/run()/stop() default implement
*/
typedef struct ztsk_timer_s{
  int tsk_type;
  int flag;     // 0x01 -act down 0x00 -trigger but not act
  int interval; // seconds
  time_t timestemp;
  // user data
  // ...
}ztsk_timer_t;

#define ZTSK_TIMER_ISACT(timer) (timer->flag & 0x01)
#define ZTSK_TIMER_SETACT(timer) (timer->flag |= 0x01)
#define ZTSK_TIMER_CLRACT(timer) (timer->flag ^= (timer->flag & 0x01))

#define ZTSK_TIMER_ISENABLE(timer) (timer->flag & 0x02)
#define ZTSK_TIMER_SETENABLE(timer) (timer->flag |= 0x02)
#define ZTSK_TIMER_CLRENABLE(timer) (timer->flag ^= (timer->flag & 0x02))

ZAPI int ztsk_timer_init();
ZAPI int ztsk_timer_fini(zoperate op_free);
ZAPI int ztsk_timer_add(ztsk_timer_t *timer);
ZAPI int ztsk_timer_trigger(ztsk_svr_t *svr);
ZAPI int ztsk_timer_update(ztsk_timer_t *timer, time_t tmstemp); // update timer->timestemp to now
ZAPI int ztsk_timer_set(ztsk_timer_t *timer, int tsk_type, int interval);
#endif
