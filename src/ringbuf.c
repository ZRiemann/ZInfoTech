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
  zmutex_uninit(&(ring->mtx));
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
  zmutex_uninit(&(ring->mtx));
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
  zmutex_lock(&(ring->mtx));
  // ZDBG("read begin: rest<%d> restx<%d> len<%d> lenx<%d>", ring->rest, ring->restx, ring->len, ring->lenx);
  // read len
  if(0 == ring->len){
    *len = 0;
	zmutex_unlock(&(ring->mtx));
    return ZENOT_EXIST;
  }
  
  read = (ring->len > *len)?(*len):(ring->len);
  memcpy(buf, ring->pr, read);
  ring->pr += read;
  ring->len -= read;
  ring->restx += read;
  
  // pr == pw set to ring->buf
  if( (0 == ring->len) && (ring->pr != ring->pw)){
    //ZDBG("### 0 == ring->len && pr!=pw###");
    ring->pr = ring->buf;
    ring->rest = ring->restx;
    ring->restx = 0;
    ring->len = ring->lenx;
    ring->lenx = 0;
  }
   // not thread safe.
  if(ring->pr == ring->pw){
    //ZDBG("### pr == pw ## reset ring buf");
    ring->pr = ring->pw = ring->buf;
    ring->rest = ring->size;
    ring->len = ring->restx = ring->lenx = 0;
  }
  *len = read;
  //ZDBG("read end: rest<%d> restx<%d> len<%d> lenx<%d>", ring->rest, ring->restx, ring->len, ring->lenx);
  zmutex_unlock(&(ring->mtx));
#if ZTRACE_RING
  ZERRC(ret);
#endif
  return ret;
}

int zring_write(zring_t* ring, const char* buf, int len){
  int ret = ZEOK;
  // NULL != ring/buf/len>0
  zmutex_lock(&(ring->mtx));
  if(ring->pw >= ring->pr){
    //=====================================================================
    // buf ... pr ... pw ... buf[size-1]
    if(ring->rest < len){
      // buf (write)... pw' ... pr ... pw (write)... buf[size-1]
      if((ring->rest + ring->restx -1) < len){// rest-1 pw not arrave pr for parallel RW
		ret = ZEMEM_INSUFFICIENT;
		//ZERRC(ret);
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
  zmutex_unlock(&(ring->mtx));
  //ZDBG("write end: rest<%d> restx<%d> len<%d> lenx<%d>", ring->rest, ring->restx, ring->len, ring->lenx);
#if ZTRACE_RING
  ZERRC(ret);
#endif
  return ret;
}

int zringt_read(zring_t* ring, char* buf, int* len){
  int ret = ZEOK;
  int rlen = ring->pw - ring->pr;
  int read = 0;

  // pr can reach pw, pw == pr means buf empty.
  if(rlen > 0){
    // buf ... pr ... pr' ... pw ... buf[size-1]
    read = (rlen < *len) ? rlen : *len;
  }else if(rlen < 0){
    // buf ... pw ... pr ... buf[size-1]
    rlen = ring->size - (ring->pr - ring->buf);
    if( rlen > 0){
      // buf ... pw ...pr ...pr' ... buf[size-1]
      read = (rlen < *len) ? rlen : *len;
    }else if(0 == rlen){
      rlen = ring->pw - ring->buf;
      if(rlen > 0){
	// buf ... pw ...pr==buf[size-1]
	read = (rlen < *len) ? rlen : *len;
	ring->pr = ring->buf;
      }else if(rlen == 0){
	// pw=buf ... pr==buf[size-1] (read empty)
	ret = ZEMEM_INSUFFICIENT;
      }else{
	// pw ... buf ... pr==buf[size-1] (impossible)
	ret = ZEMEM_OUTOFBOUNDS;
      }
    }else{
      //buf ... pw ... buf[size-1] ... pr(impossible)
      ret = ZEMEM_OUTOFBOUNDS;
    }
  }else{
    // buf ... pr==pw ... buf[size-1] read empty
    return ZEMEM_INSUFFICIENT;
  }

  if(read > 0){
    memcpy(buf, ring->pr, read);
    ring->pr += read;
  }
  *len = read;
#if ZTRACE_RING
  ZERRCX(ret);
  if(ZEOK == ret){
    static size_t sr = 0;
    ZDBG("read: %lld", sr += read);
  }
#endif
  return ret;
}

int zringt_write(zring_t* ring, const char* buf, int* len){
  int ret = ZEOK;
  int rlen = ring->pw - ring->pr;
  int wlen = 0;
  int write = 0;
  int rest = 0;

  if(rlen > 0){
    // buf ... pr ... pw ... buf[size-1]
    wlen = ring->size - (ring->pw - ring->buf);
    if(0 < wlen){
      // buf ... pr ... pw ...pw'...buf[size -1]
      // buf ... pr ... pw ...pw'==buf[size-1]
      write = (wlen < *len) ? wlen : *len;
    }else if(0 == wlen){
      if(ring->pr > ring->buf){
	// buf ... pr ... pw==buf[size-1]
	wlen = ring->pr - ring->buf -1; // pw not reach pr
	write = (wlen < *len) ? wlen: *len;
	ring->pw = ring->buf;
      }else{
	// buf==pr ... pw==buf[size-1]
	ret = ZEMEM_INSUFFICIENT;
      }
    }else{
      // buf ... pr ... buf[size-1] ... pw (impossible)
      ret = ZEMEM_OUTOFBOUNDS;
    }
  }else if(rlen < 0){
    wlen = -rlen;
    --wlen; // not reach pr
    if( wlen > 0){
      // buf ... pw ... pr ... buf[size-1]
      write = (wlen < *len) ? wlen : *len;
    }else{
      // buf ... pw pr ... buf[size-1]
      ret = ZEMEM_INSUFFICIENT;
    }
  }else{
    // buf ... pw=pr ... buf[size-1] init status or read empty
    wlen = ring->size - (ring->pw - ring->buf);
    if(wlen > 0){
      write = (wlen < *len) ? wlen : *len;
    }else if(0 == wlen){
      // buf ... pw==write = (wlen < *len) ? wlen : *len;
      ring->pw = ring->buf;
      write = (ring->size < *len) ? ring->size : *len;
    }else{
      ret = ZEMEM_OUTOFBOUNDS;
    }
  }

  if(write > 0){
    memcpy(ring->pw, buf, write);
    ring->pw += write;
  }
  *len = write;
  
#if ZTRACE_RING
  ZERRCX(ret);
  if(ZEOK == ret){
    static size_t sw = 0;
    ZDBG("write: %lld", sw += write);
  }
#endif
  return ret;
}

int zringt_strread(zring_t* ring, char* buf, int* len){
  int ret = ZEOK;
  int rlen = ring->pw - ring->pr;
  int read = 0;
  int rlenx = 0;
  // pr can reach pw, pw == pr means buf empty.
  if(rlen > 0){
    // buf ... pr ... pr' ... pw ... buf[size-1]
    rlen = strlen(ring->pr)+1; // contain '\0'
    rlenx = ring->size - (ring->pr - ring->buf);
    rlen = (rlen < rlenx) ? rlen : rlenx;
    read = (rlen < *len) ? rlen : *len;
  }else if(rlen < 0){
    // buf ... pw ... pr ... buf[size-1]
    rlen = ring->size - (ring->pr - ring->buf);
    if( rlen > 0){
      // buf ... pw ...pr ...pr' ... buf[size-1]
      rlenx = ring->size - (ring->pr - ring->buf);
      rlen = strlen(ring->pr)+1;
      rlen = (rlenx < rlen) ? rlenx : rlen;
      read = (rlen < *len) ? rlen : *len;
    }else if(0 == rlen){
      rlen = ring->pw - ring->buf;
      if(rlen > 0){
	// buf ... pw ...pr==buf[size-1]
	ring->pr = ring->buf;
	rlenx = strlen(ring->pr)+1;
	rlen = (rlen < rlenx)? rlen : rlenx;
	read = (rlen < *len) ? rlen : *len;
      }else if(rlen == 0){
	// pw=buf ... pr==buf[size-1] (read empty)
	ret = ZEMEM_INSUFFICIENT;
      }else{
	// pw ... buf ... pr==buf[size-1] (impossible)
	ret = ZEMEM_OUTOFBOUNDS;
      }
    }else{
      //buf ... pw ... buf[size-1] ... pr(impossible)
      ret = ZEMEM_OUTOFBOUNDS;
    }
  }else{
    // buf ... pr==pw ... buf[size-1] read empty
    return ZEMEM_INSUFFICIENT;
  }

  if(read > 0){
    memcpy(buf, ring->pr, read);
    ring->pr += read;
  }
  *len = read;
  
#if ZTRACE_RING
  ZERRCX(ret);
  if(ZEOK == ret){
    static size_t sr = 0;
    ZDBG("read: %lld", sr += read);
  }
#endif
  return ret;
}

int zringt_strwrite(zring_t* ring, const char* buf, int* len){
  return zringt_write(ring, buf, len);
}
