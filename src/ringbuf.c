#include "export.h"
#include <zit/base/ringbuf.h>
#include <zit/base/trace.h>
#include <zit/thread/mutex.h>
#include <stdlib.h>
#include <string.h>

/*
  ring buf: 0 1 2 3 ... n n+1 n+2 ... size-1
            |--pw/pr

            |-- restx --| |--- len ---| |-- rest ---|
  ring buf: 0 1 2 3 ... m m+1 m+2 ... n n+1 n+2 ... size-1
                          |--pr         |--pw

            |--- lenx --| |-- restx --| |---- len --|
  ring buf: 0 1 2 3 ... m m+1 m+2 ... n n+1 n+2 ... size-1
                          |--pw         |--pr

  if(size-pw < write len) return ZEMEM_INSUFFICIENT;
*/
zring_t* zring_create(int size){
  int ret = ZEOK;
  zring_t* ring =(zring_t*)malloc(sizeof(zring_t));
  if(NULL != ring){
    ring->rest = ring->size = size;
    ring->len = ring->restx = ring->lenx = 0;
    if(NULL == (ring->buf = (char*)malloc(size))){
      ret = ZEMEM_INSUFFICIENT;
    }else{
      ring->pw = ring->pr = ring->buf;
      ret = zmutex_init(&(ring->mtx));
    }
  }else{
    ret = ZEMEM_INSUFFICIENT;
  }
  
#if ZTRACE_RING
  ZERRC(ret);
#else
  ZERRCX(ret);
#endif
  return ring;
}
void zring_destroy(zring_t* ring){
  // NULL != ring
  free(ring->buf);
  free(ring);
}
int zring_init(zring_t* ring, int size){
  int ret = ZEOK;
  ring->rest = ring->size = size;
  ring->len = ring->restx = ring->lenx = 0;
  if(NULL == (ring->buf = (char*)malloc(size))){
    ret = ZEMEM_INSUFFICIENT;
  }else{
    ring->pw = ring->pr = ring->buf;
    ret = zmutex_init(&(ring->mtx));
  }
#if ZTRACE_RING
  ZERRC(ret);
#else
  ZERRCX(ret);
#endif
  return ret;
}

int zring_uninit(zring_t* ring){
  int ret = ZEOK;
  // NULL != ring
  free(ring->buf);
#if ZTRACE_RING
  ZERRC(ret);
#endif
  return ret;
}

int zring_read(zring_t* ring, char* buf, int* len){
  int ret = ZEOK;
  int read = 0;
  char* pb = buf;
  // read len
  if(0 == ring->len){
    return ZENOT_EXIST;
  }
  
  read = (ring->len > *len)?(*len):(ring->len);
  memcpy(buf, ring->pr, read);
  ring->pr += read;
  ring->len -= read;
  ring->restx += read;
  // pr == pw set to ring->buf
  if(0 == ring->len){
    ring->pr = ring->buf;
    ring->rest = ring->restx;
    ring->restx = 0;
    ring->len = ring->lenx;
    ring->lenx = 0;
  }
  
  if(ring->pr == ring->pw){
    ring->pr = ring->pw = ring->buf;
    ring->rest = ring->size;
    ring->len = ring->restx = ring->lenx = 0;
  }
  *len = read;
#if ZTRACE_RING
  ZERRC(ret);
#endif
  return ret;
}

int zring_write(zring_t* ring, char* buf, int len){
  int ret = ZEOK;
  // NULL != ring/buf/len>0
  if(ring->pw >= ring->pr){
    //=====================================================================
    // buf ... pr ... pw ... buf[size-1]
    if(ring->rest < len){
      // buf (write)... pw' ... pr ... pw (write)... buf[size-1]
      if((ring->rest + ring->restx) < len){
	ret = ZEMEM_INSUFFICIENT;
      }else{
	// write rest
	memcpy(ring->pw, buf, ring->rest);
	buf += ring->rest; // buf (write rest)...buf'...(len -rest)
	len -= ring->rest;
	ring->len += ring->rest;
	ring->rest = 0;
	ring->pw = ring->buf;
	// write restx
	memcpy(ring->pw, buf, len);
	ring->restx -= len;
	ring->pw += len;
	ring->lenx += len;
      }
    }else{
      //------------------------------------------------------------------
      // buf ... pr ... pw (write)... pw' ... buf[size-1]
      ring->rest -= len;
      memcpy(ring->pw, buf, len);
      ring->pw += len;
      ring->len +=len;
      if( 0 == ring->rest){ // if end position goto start position.
	ring->pw = ring->buf;
	ring->lenx = 0;
	//ring->restx = ring->size - ring->len;
      }
    }
  }else{
    //===================================================================
    // buf ... pw ... pr ... buf[size-1]
    if(ring->restx >=  len){
      memcpy(ring->pw, buf, len);
      ring->pw += len;
      ring->restx -= len;
      ring->lenx +=len;
    }else{
      ret = ZEMEM_INSUFFICIENT;
    } 
  }
  
#if ZTRACE_RING
  ZERRC(ret);
#endif
  return ret;
}

// thread safe
int zringt_read(zring_t* ring, char* buf, int* len){
  int ret = ZEOK;
  zmutex_lock(&(ring->mtx));
  ret = zring_read(ring, buf, len);
  zmutex_unlock(&(ring->mtx));
  return ret;
}

int zringt_write(zring_t* ring, char* buf, int len){
  int ret = ZEOK;
  zmutex_lock(&(ring->mtx));
  ret = zring_write(ring, buf, len);
  zmutex_unlock(&(ring->mtx));
  return ret;
}


