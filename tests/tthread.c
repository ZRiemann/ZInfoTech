#include <zit/base/trace.h>
#include <zit/thread/semaphore.h>
#include <zit/thread/mutex.h>
#include <zit/thread/thread.h>
#include <zit/thread/jet.h>
#include <zit/base/time.h>
#include <zit/base/error.h>
#include <zit/base/queue.h>
#include <zit/base/ringbuf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tthread.h"

//#define USE_RINGT

//static FILE* fw = NULL;
//static FILE* fr = NULL;
static size_t zg_sw = 0;
static size_t zg_sr = 0;
static char zg_wbuf[] = "aaaaaaaa";
static char zg_rbuf[8] = {0};

void ztst_jet();
void ztst_queue();
void ztst_ring();
void ztst_ring_base();
void ztst_ring_str(); // test ring string read and write.

void ztst_thread(){
  //ztst_semaphore();
  //ztst_mutex();
  //ztst_thrctl();
  //ztst_thrxctl();
  //ztst_queue();
  //ztst_jet();
  //ztst_ring();
  //ztst_ring_base();
  ztst_ring_str();
}

int ztsk_dummy(zvalue_t user, zvalue_t hint){
  return ZEOK; // dummy
}

int zact_producer(zvalue_t user, zvalue_t hint){
  zring_t* ring = (zring_t*)user;
  /*int i = 0;
  char aaa[9] = {0};
  int lena = (rand()%8)+1;
  int delay = (rand()%500)+500;
  for(i = 0; i < lena; i++){
    aaa[i] = (rand()%26)+'a';
    }*/
  int lena = 8;
#ifdef USE_RINGT
  if(ZEOK == zring_write(ring, zg_wbuf, &lena) ){
#else
  if(ZEOK == zringt_write(ring, zg_wbuf, lena) ){
#endif
    //fwrite(aaa, sizeof(char), lena, fw);
    //ZDBG("write: %s", aaa);
    zg_sw += lena;
  }
  //zsleepms(delay);
  zjet_assign(hint);
  return ZEOK;
}

int zact_customer(zvalue_t user, zvalue_t hint){
  zring_t* ring = (zring_t*)user;
  /*char buf[128] = {0};
  int len = (rand()%16)+1;
  int delay = (rand()%500)+500;
  */
  int len = 8;
#ifdef USE_RINGT
  if( ZEOK == zring_read(ring, zg_rbuf, &len) ){
#else
  if( ZEOK == zringt_read(ring, zg_rbuf, &len) ){
#endif
    //fwrite(buf, sizeof(char), len, fr);
    //ZDBG("read: %s", buf);
    zg_sr += len;
  }
  //zsleepms(delay);
  zjet_assign(hint);
  return ZEOK;
}

int zact_ringstr_customer(zvalue_t user, zvalue_t hint){
  zring_t* ring = (zring_t*)user;
  //  int delay = (rand()%500)+500;
  static char buf[128] = {0};
  static int len = 128;
  len = 128;
  if(ZEOK == zring_strread(ring, buf, &len)){
    buf[len] = 0;
    ZDBG("read<%d>: %s",len, buf);
  }
  //zsleepms(delay);
  if(NULL != hint){
    zjet_assign(hint);
  }
  return ZEOK;
}

int zact_ringstr_producer(zvalue_t user, zvalue_t hint){
  int ret = ZEOK;
  static int cnt = 0;
  static char buf[128] = {0};
  int len = 0;
  int write = 0;
  //  int delay = (rand()%500)+500;
  zring_t* ring = (zring_t*)user;

  sprintf(buf, "string %d", cnt++);
  len = write = strlen(buf)+1;
  do{
    if(ZEOK == zring_strwrite(ring, buf, &write)){
      len -= write;
      write = len;
      zsleepms(0);
      if(0 != len){
	ZDBG("rest %d need to write.", len);
      }
    }
    // wait zjet eixt semaphore for exit...
  }while(len);
  ZDBG("write: %s", buf);
  //zsleepms(delay);
  if(NULL != hint){
    zjet_assign(hint);
  }
  
  return ret;
}

void ztst_ring_str(){
  //  int ret = ZEOK;
  // int len = 0;
  //int write = 0;
  //int lenx = 0;
  //int i = 0;
  ztsk_t tskp; // roducer
  ztsk_t tskc; // customer
  zring_t ring;
  
  ZDBG("testing ring string read and write...");
  zring_init(&ring, 32);
  //======================================================
  srand(time(NULL));

  tskp.user = &ring;
  tskp.hint = &tskp;
  tskp.mode = ZTSKMD_SEQUENCE;
  tskp.misid = 0;
  tskp.act = zact_ringstr_producer;
  tskp.free = ztsk_dummy;
  
  tskc.user = &ring;
  tskc.hint = &tskc;
  tskc.mode = ZTSKMD_SEQUENCE;
  tskc.misid = 1;
  tskc.act = zact_ringstr_customer;
  tskc.free = ztsk_dummy;

  zjet_assign(&tskp);
  zjet_assign(&tskc);

  zsleepms(50000);
  
  tskc.hint = tskp.hint = NULL; // stop dead loop
  zsleepms(500); // wait task end
  zring_uninit(&ring);
}

void ztst_ring_base(){
  zring_t ring;
  char wbuf[] = "abcdefghijklmnopqrstuvwxyz";
  char rbuf[27] = {0};
  int rlen = 27;
  zring_init(&ring, 26);
  zdbg("ring init(000): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);
  // write 26
  rlen = 26;
  zringt_write(&ring, wbuf, rlen);
  zdbg("ring write(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr); 
  // read 26
  rlen = 27;
  memset(rbuf, 0, sizeof(rbuf));
  zringt_read(&ring, rbuf, &rlen);
  zdbg("ring read(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);

  // write 13
  rlen = 13;
  zringt_write(&ring, wbuf, rlen);
  zdbg("ring write(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);

  // read 8
  rlen = 8;
  memset(rbuf, 0, sizeof(rbuf));
  zringt_read(&ring, rbuf, &rlen);
  zdbg("ring read(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);

  // write
  rlen = 20;
  zringt_write(&ring, wbuf, rlen);
  zdbg("ring write(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);
  
  // read 8
  rlen = 27;
  memset(rbuf, 0, sizeof(rbuf));
  zringt_read(&ring, rbuf, &rlen);
  zdbg("ring read(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);

  // read 8
  rlen = 27;
  memset(rbuf, 0, sizeof(rbuf));
  zringt_read(&ring, rbuf, &rlen);
  zdbg("ring read(%02d): lenx<%d> restx<%d> len<%d> rest<%d> pw<%p> pr<%p>", rlen, ring.lenx, ring.restx, ring.len, ring.rest, ring.pw, ring.pr);

  // uninit
  zring_uninit(&ring);
}

void ztst_ring(){
  zring_t ring;
  ztsk_t tskp; // roducer
  ztsk_t tskc; // customer

  //fw = fopen("ringwrite.txt", "wb");
  //fr = fopen("ringread.txt", "wb");
  srand(time(NULL));
  zring_init(&ring,256);

  zjet_init();
  zjet_run();

  tskp.user = &ring;
  tskp.hint = &tskp;
  tskp.mode = ZTSKMD_SEQUENCE;
  tskp.misid = 0;
  tskp.act = zact_producer;
  tskp.free = ztsk_dummy;
  
  tskc.user = &ring;
  tskc.hint = &tskc;
  tskc.mode = ZTSKMD_SEQUENCE;
  tskc.misid = 1;
  tskc.act = zact_customer;
  tskc.free = ztsk_dummy;

  zjet_assign(&tskp);
  zjet_assign(&tskc);

  //zsleepms(50);
  zsleepsec(5);
  //zjet_stop(0);
  //zjet_uninit();
  zring_uninit(&ring);
#ifdef USE_RINGT
  ZDBG("zring*T* total write<%d> read<%d>", zg_sw, zg_sr);
#else
  ZDBG("zring total write<%d> read<%d>", zg_sw, zg_sr);
#endif
  //fclose(fr);
  //fclose(fw);
}

int zcmp_int(zvalue_t v1, zvalue_t v2){
  int ret = ZEQUAL;
  int i1 = 0;
  int i2 = 0;
  int sub = 0;
  ZCONVERT(v1,i1);
  ZCONVERT(v2,i2);

  sub = i1 - i2;
  if(0 < sub){
    ret = ZGREAT;
  }else if(0 > sub){
    ret = ZLITTLE;
  }
  return ret;
}

int zact_intque(zvalue_t user, zvalue_t hint){
  int* index = (int*)hint;
  zmsg("queue[%03d] = %d", *index, user);
  ++(*index);
  return ZEOK;
}
void ztst_queue(){
  zcontainer_t que = NULL;
  int data = 0;
  zvalue_t v;
  int ret = ZEOK;
  //int cnt = 0;
  if(ZEOK != (ret = zque_create(&que))){
    ZERRC(ret);
    return;
  }
  ZCONVERT(v, data);data++;
  zque_push(que, v);
  ZCONVERT(v, data);data++;
  zque_pushfront(que, v);
  ZCONVERT(v, data);data++;
  zque_push(que, v);
  ZCONVERT(v, data);data++;
  zque_pushfront(que, v);
  
  zque_popback(que, &v); //CAUTION: *_pop*(que, (zvalue_t*)&data); CAUSE FLAGMENT FAULT. 
  zque_pop(que, &v);
  //cnt = 0;
  zque_destroy(que,NULL);

  zdbg("test ZCONVERT(dest, src) convert 'src' data to 'dest' data...");
  data = 987654321;
  zdbg("before ZCONVERT(): data = %d; v = %d", data, v);
  ZCONVERT(v, data);
  zdbg("after ZCONVERT(): data = %d; v = %d", data, v);
  data = 0;
  zdbg("before ZCONVERT(): data = %d; v = %d", data, v);
  ZCONVERT(data, v);
  zdbg("after ZCONVERT(): data = %d; v = %d", data, v);
}

void ztst_jet(){
  zjet_init();
  zjet_run();
  zsleepsec(3);
  zjet_stop(0);
  zjet_uninit();
}
void ztst_mutex(){
  zmutex_t mtx;
  zmutex_init(&mtx);
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
  zmutex_uninit(&mtx);
#ifdef ZSYS_POSIX
  zmutex_lock(&mtx);
  zmutex_unlock(&mtx);
#endif
}
void ztst_semaphore(){
  zsem_t sem;
  //zsem_t sem1;
  //ZMSG("test normal...");
  zsem_init(&sem,0);
  zsem_post(&sem);
  zsem_wait(&sem, ZINFINITE);
  zsem_wait(&sem, 1000);
  zsem_uninit(&sem);
  
  //ZMSG("test sem destroyed...");
  //zsem_post(&sem1);
  //zsem_init(&sem1, 0);
  //zsem_wait(&sem1,100);
  //zsem_wait(&sem1,ZINFINITE);
}

zthr_ret_t ZCALL zproc_thr1(void* param){
  zthr_t* attr = (zthr_t*)param;
  int i = 0;
  
  ZDBG("thread[%s] beging...", attr->name);
  for(;i<10;i++){
    ZDBG("thread %s run %d...", attr->name, i);
    zsleepms(500);
  }
  ZDBG("thread[%s] end.", attr->name);

  return (zthr_ret_t)ZEOK;
}

zthr_ret_t ZCALL zproc_thr2(void* param){
  int ret = ZEOK;
  zthr_t* attr = (zthr_t*)param;
  //void* user_param = attr->param; // for user parameter
  //ZDBG("thread[%s] running...");
  if(ZEOK != zthreadx_procbegin(attr)){
    zthreadx_procend(attr, ret);
    return (zthr_ret_t)ZEFAIL;
  }
  while( ZETIMEOUT == zsem_wait(&(attr->exit), 500)){
    // loop working...
    ZDBG("%s working...", attr->name);
  }
  zthreadx_procend(attr, ret);
  //ZDBG("thread[%s] exit now.");
  return (zthr_ret_t)ZEOK;
}

void ztst_thrctl(){
  int i = 0;
  zthr_id_t id;
  zthr_t attr;
  sprintf(attr.name, "thr[0]");
  ZDBG("testing ztst_thrctl()...");
  zthread_create(&id, zproc_thr1, (void*)&attr);
  // main loop
  for(i=0; i< 10; i++){
    ZDBG("main loop %d...", i);
    zsleepms(500);
  }
  // cancel thread
  zthread_cancel(&id);
  zthread_join(&id);
}

void ztst_thrxctl(){
  zthr_t zthr;
  zthr_t zthr1;
  zthr_t zthr2;
  zthr.name[0] = 0; // use default thread name.
  zthr1.name[0] = 0;
  zthr2.name[0] = 0;

  zthreadx_create(&zthr, zproc_thr2);
  zthreadx_create(&zthr1, zproc_thr2);
  zthreadx_create(&zthr2, zproc_thr2);
  zsleepsec(3);
  zthreadx_cancelall(); // cancel all threads
  zthreadx_joinall(); // join all threads
  //zthreadx_cancel(&zthr);
  //zthreadx_join(&zthr);
}
