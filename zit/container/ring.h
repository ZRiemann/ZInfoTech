#ifndef _ZCONTAINER_RING_H_
#define _ZCONTAINER_RING_H_
/**
 * @file A ring buffer for read/write parallel
 */

/**
 * @brief Ring buffer structure
 *
 * ring[0][1][2][3][4][5][6][7][8][9]...[size-1] create or empty status
 *      \- begin,end
 * ring[0][1][2][3][4][5][6][7][8][9]...[size-1] begin < end status
 *         \- begin          \- end
 * ring[0][1][2][3][4][5][6][7][8][9]...[size-1] end < begin status
 *         \- end            \- begin
 * ring[0][1][2][3][4][5][6][7][8][9]...[size-1] full buffer status
 *            \  \- begin
 *             \--- end
 */
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <zit/base/error.h>

typedef struct zring_s{
    zatm32_t size; /** ring buffer size */
    zatm32_t wpos; /** write position */
    zatm32_t rpos; /** read position */
    char *buf; /** ring buffer, user set or allocate in init*/
    int capacity; /** ring capacity 1024<<n */
    char free; /** 0: not free bufer; 1:free buffer*/
    char full; /** 0:not full; 1:full*/
    int mask; /** end mask */
}zring_t;

zinline zerr_t zring_init(zring_t *ring){
    int zerr_t = ZEOK;
    if(!ring){
        /* allocate memory*/
        ring->size = ring->size < 1024 ? 4096 : ring->size;
        ring->ring = (char*)calloc(1, size);
        if(!ring->buf){
            ret = ZEMEM_INSUFFICIENT;
        }
        ring->flag = 1;
    }
    ring->wpos = 0;
    ring->rpos = ring->size;;
    ring->full = 0;
    ring->mask = ring->size -1;
    return ret;
}

zinline zerr_t zring_fini(zring_t *ring){
    if(ring->flag){
        free(ring->buf);
    }
    return ZEOK;
}

zinline int zring_get_wsize(zring_t *ring){
    return ring->capacity - ring->size;
}

zinline void zring_get_wbuf(zring_t *ring, char **buf, int *size){
    int wsize = 0;

    if(ring->full){
        buf = NULL;
        return;
    }
    *buf = &ring->buf[wpos];
    wsize = ring->rpos > ring->wpos ? ring->rpos - ring->wpos : ring->size - ring->wpos;
    if(wsize > *size){
        wsize = *size;
    }
    ring->wpos += wsize;
    ring->wpos &=ring->mask;
    if(ring->rpos == ring->wpos){
        ring->full = 1;
    }
}

zinline void zring_get_rbuf(zring_t *ring, char **buf, int *size){
    if(ring->rpos == ring->wpos && !ring->full){
        *buf = NULL;
        return;
    }
    *buf = &ring->buf[rpos];
    *size = ring->wpos > ring->rpos ? rong->wpos - ring->rpos : ring->size - ring->rpos;
}

zinline void zring_pop_rbuf(zring_t *ring, int size){
    ring->rpos += size;
    ring->rpos &= rong->mask;
    ring->full = 0;
}

#if 0

zinline zerr_t zring_write_packet(zring_t *ring, char *buf, int size){
    char *wbuf = NULL;
    char *wbuf1 = NULL;
    int size = 0;
    int size1 = 0;
    do{
        zring_get_wbuf(ring, &wbuf, &size);
        if(wbuf)
    }while(0);
}

zinline zerr_t zring_read_packet(zring_t *ring, char **buf, int *size){
    if(ring->rpos == ring->wpos && !ring->full){
        *buf = NULL;
        return;
    }
    if(ring->wpos > ring->rpos){
        *buf = &rong->buf[rpos];
        *size = ring->wpos - ring->rpos;
    }else{
        
    }
}

zinline zerr_t zring_pop_block(zring_t *ring, size){
    
}
#endif /* if 0 */
#endif
