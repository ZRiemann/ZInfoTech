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
  zmtx_t mtx; ///< thread safe lock
}zring_t;

ZEXP zring_t* zring_create(int size);
ZEXP void zring_destroy(zring_t* ring);
ZEXP int zring_init(zring_t* ring, int size);
ZEXP int zring_uninit(zring_t* ring);
ZEXP int zring_read(zring_t* ring, char* buf, int* len);
ZEXP int zring_write(zring_t* ring, char* buf, int len);
// thread safe
ZEXP int zringt_read(zring_t* ring, char* buf, int* len);
ZEXP int zringt_write(zring_t* ring, char* buf, int len);
// extern for 0 copy, operate ring->pw/pr/mtx/... directory
#if 0
ZEXP int zring_getread(zring_t* ring, char** buf, int* len);
ZEXP int zring_getwrite(zring_t* ring, char** buf, int* len):
#endif
  
#ifdef __cplusplus
}
#endif
#endif
