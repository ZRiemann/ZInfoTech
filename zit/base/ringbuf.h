#ifndef _ZBASE_RINGBUF_H_
#define _ZBASE_RINGBUF_H_

#include <zit/base/platform.h>
#include <zit/thread/thread_def.h>

#ifdef __cplusplus
extern "C" {
#endif
  
typedef struct zring_s{
  int size; ///< ring buf size;
  int rest; ///< rest buf size for write
  int len; ///< buf for read length
  int restx; ///< extend rest buf for write
  int lenx; ///<extend buf for read
  char* buf; ///< ring buffer pointer
  char* pw; ///< write position
  char* pr; ///< read position
  zmtx_t mtx; ///< lock; CAUTION: not thread safe(use rwbuf for parallel read/wrte)
}zring_t;

ZEXP zring_t* zring_create(int size);
ZEXP void zring_destroy(zring_t* ring);
ZEXP int zring_init(zring_t* ring, int size);
ZEXP int zring_uninit(zring_t* ring);
  // mtx and ring
ZEXP int zringt_read(zring_t* ring, char* buf, int* len);
ZEXP int zringt_write(zring_t* ring, const char* buf, int len);
  // no mtx and ring
ZEXP int zring_read(zring_t* ring, char* buf, int* len);
ZEXP int zring_write(zring_t* ring, const char* buf, int* len);
  // block read/write
  //ZEXP int zring_blkread(zring_t* ring, char* buf, int* len);
  //ZEXP int zring_blkwrite(zring_t* ring, char* buf, int* len);
  // string read/write
ZEXP int zring_strread(zring_t* ring, char* buf, int* len);
ZEXP int zring_strwrite(zring_t* ring, const char* buf, int* len);

#if 0
  // for 0 mem copy
ZEXP int zring_getread(zring_t* ring, char** buf, int* len);
ZEXP int zring_getwrite(zring_t* ring, char** buf, int* len):
#endif
  
#ifdef __cplusplus
}
#endif
/**@file zit/base/ringbuf.h
   @brief ring buffer milti implements
   @note
   
   test:   tests/tthread.c:ztst_ring() 5 seconds parallel read and write 
   fun:    zring_write/read()       zringt_write/read()
   round1: 159731400/159731216       114433632/114433632
   round2: 163016456/163016448       120403688/120403480
   round3: 160442568/160442320       116188104/116187848
   round4: 161647152/161646904       134485204/134484984
   round5: 163004831/163004584       110779208/110779184
   ------------------------------------------------------
   total:  807842407/807841472       596289872/596289128
   average:161568481/161568294       119257974/119257825
   times:  1.35                      1
*/
#endif
