/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#if 0
#include "export.h"
#include <zit/utility/tracering.h>
#include <zit/base/type.h>
#include <zit/base/ringbuf.h>
#include <zit/thread/jet.h>
#include <zit/thread/mutex.h>
#include <stdlib.h>
#include <string.h>

typedef struct ztracering_s{
  zring_t ring;
  ztsk_t tsk;
  ztrace trace;
  void* user; ///< hint for trace user
  char msg[ZTRACE_BUF_SIZE]; ///< +1
}ztracering_t;

static ztracering_t* zg_tracering;

static int ztracering(int level, void* user, const char* msg){
  char c = (char)level + 'a';
  int len = 1;
  int len1 = strlen(msg)+1;
  if(ZRUN == zjet_getstatus()){
    // trace background
    zmutex_lock(&(zg_tracering->ring.mtx));
    if((ZEOK == zring_write(&(zg_tracering->ring), &c, &len))\
       && (ZEOK == zring_write(&(zg_tracering->ring), msg, &len1)))zjet_assign(&(zg_tracering->tsk));
    zmutex_unlock(&(zg_tracering->ring.mtx));
  }else{
    // trace directory
    zg_tracering->trace(level, user, msg);
  }
  return ZEOK;
}

static int ztracering_act(zvalue_t user, zvalue_t hint){
  int len = ZTRACE_BUF_SIZE;
  char* msg = zg_tracering->msg;
  if( ZEOK == zring_strread(&(zg_tracering->ring), msg, &len)){
    // msg = level + trace string
    zg_tracering->trace((*msg - 'a'), zg_tracering->user, (msg+1)); 
  }
  return ZEOK;
}

int ztracering_init(ztrace trace, void* user){
  int ret = ZEOK;
  if(NULL == zg_tracering){
    zg_tracering = (ztracering_t*)malloc(sizeof(ztracering_t));
    if(NULL == zg_tracering){
      ret = ZEMEM_INSUFFICIENT;
    }else{
      ztrace_reg(ztracering, user);
      zring_init(&(zg_tracering->ring), 4096*100);
      zg_tracering->trace = trace;
      zg_tracering->user = user;
      zg_tracering->tsk.user = &(zg_tracering->ring);
      zg_tracering->tsk.mode = ZTSKMD_SEQUENCE;
      zg_tracering->tsk.misid = 0;
      zg_tracering->tsk.hint = zg_tracering->msg;
      zg_tracering->tsk.act = ztracering_act;
      zg_tracering->tsk.free = NULL;
    }
  }
  return ret;
}

int ztracering_uninit(){
  if(NULL != zg_tracering){
    zring_uninit(&(zg_tracering->ring));
    free(zg_tracering);
  }
  return ZEOK;
}

#endif
