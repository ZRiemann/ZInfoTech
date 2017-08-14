#ifndef _Z_STATISTIC_H_
#define _Z_STATISTIC_H_
/**
 * @file zit/statistic/statistic.h
 * @brief user definition process running time statistic
 * @details
 *  - Statistic callback zoperate(ZOP_ARG)
 *    param in pointer to tatistic data struct.
 *    param out reserve
 *    param hint pointer to statistic operate ZOP_STATISTIC_*.
 *    Statistic operate can user define for their own requirement;
 *  - Support in zit
 *    -# queue_1r1w serial queue @see zit/container/queue_1r1w.h
 *  - Functional requirement (Design pattern)
 *    -# Every module support it self statistic busyness
 *    -# Support a root statistic list to collect all statistic infomation
 *    -# Support dynamic add and delete statistic module.
 *  - Framework
 *    (root)
 *      |- base info
 *      |    |- start time, run time.
 *      |    \- version and so on.
 *      |- threads
 *      |    |- zstat_thr
 *      |- memory
 *      \- module statistic
 *  - resource + operaate and thread + stack map
 *
 *           fn[0] fn[1] ... fn[n]
 *    res[0]   X     O   ...
 *    res[1]   O     O   ... O
 *    ...      ...
 *           stack[0] stack[1] ... stack[n]
 *    thr[0] r0f1     r1f0
 *    thr[1] r1f1     r1fn
 *  - test case on ubuntu 17 (vm) it's cost 200 nanosecond per function hits;
 *    make/release zit_test queue_1r1w
 *    set test/tstatistic.h ZUSE_STATISTIC 0 *CLOSE* statistic 10,000,000 push
 *                                     first      second     third    average  nanosecond
 *    base check push pop interval   : 0.853214 , 0.911907 , 0.931628 0.898916 90  0
 *    set test/tstatistic.h ZUSE_STATISTIC 1 *OPEN* statistic
 *    only hots_hits interval        : 1.083357 , 1.186184 , 1.037301 1.102281 110 20
 *    hots_hits  + tid               : 1.898774 , 1.854854 , 1.806907 1.853512 185 95
 *    hots_hits + tid + itd_stack    : 2.083396 , 1.984417 , 1.997139 2.021651 202 112
 *    (above) + begin + end          : 2.709761 , 2.657957 , 2.708383 2.692034 269 179
 *    (above) + interval             : 2.765037 , 2.736315 , 2.799399 2.766917 276 186
 *    (above) + hitspot + real_time  : 2.944528 , 2.888231 , 2.871388 2.901382 290 200
 *
 *    test case on Windows 7
 *    first      second     third    average   nanosecond
 *    base: 0.695355 , 0.584226 , 0.505325 0.505328  50   0
 *    hots: 4.810858 , 4.790505 , 7.787826 4.787829  479  429
 */
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <zit/base/time.h>
#include <zit/thread/thread.h>
/**
 * @brief A object function call counter and frequency recode
 */
typedef struct zstatistic_hotspot_s{
    zatm64_t hits; ///< hits counts
    zatm64_t total_time; ///< total time us;
    zatm_t hots_idx; ///< last hotspot index
    ztime_stamp_t hotspot[256]; ///< hotspot vector
    uint64_t real_time[256]; ///< function real time
    zatm_t stack_idx; /// thread stack idx
    uint32_t tid_stack[256]; ///< thread call stack [1-push,0-pop | tid_t]
}zstat_hots_t;

typedef struct zstatistic_node_s{
    zstat_hots_t *hots;
    ztime_stamp_t begin;
    ztime_stamp_t end;
    uint64_t interval;
    int idx;
    ztid_t tid;
}zstat_node_t;

ZAPI zerr_t zstatistic_init(zstat_hots_t ***hots, int n, int *size);
ZAPI zerr_t zstatistic_fini(zstat_hots_t ***hots, int n, int *size);
ZAPI void  zstatistic_push_hots(zstat_hots_t **hots, zstat_node_t *node, int sri, int sfi);
ZAPI void zstatistic_pop_hots(zstat_node_t *node);
ZAPI void zstatistic_dump_file(zstat_hots_t **hots, int n, int *size, const char* path);

#if 0 // sample
// zit_statistic.h
zstat_hots_t** g_hots_res;

#define SRI_GLOBAL 0 // statistic resource index global function
#define SRI_A 1 // statistic resource index class A functions
#define SRI_B 2 // statistic resource index class B functions
#define SRI_SIZE 3

#define SFI_GLOBAL 0

#define SFI_A_fa 0 // statistic function index A::fa
#define SFI_A_fa1 1 // statistic function index A::fa1
#define SFI_A_SIZE 3

#define SFI_B_fb 0
#define SFI_B_fb1 1
#define SFI_B_SIZE 2

extern zstat_hots_t **g_stat_hots; // try use singleton pattern

// zit_class.h
#include "zit_statistic.h"
class Hots{
    zstat_node_t node;
public:
    Hots(int sri, int sfi){
        zstatistic_push_hots(g_stat_ress, &node, sri, sfi);
    }

    ~Hots(){
        zstatistic_push_hots(&node);
    }
};

class A{
public:
    fa(...){
        Hots hots(SRI_A, SFI_A_fa);
        fa1(...);
    }

    fa1(...){
        Hots hots(SRI_A, SFI_A_fa1);
        //...
    }
};

class B{
public:
    fb(...){
        Hots hots(SRI_B, SFI_B_fb);
        // ...
    }

    fb1(...){
        Hots hots(SRI_B, SFI_B_fb);
    }
};

// main.cpp
#include "zit_statistic.h"
zstat_res_t **g_stat_res; // try use singleton pattern
void main(){
    int hots[] = {SFI_GLOBAL, SFI_A_SIZE, SFI_B_SIZE};
    zstatistic_init(&g_stat_res, SRI_SIZE, hots);
    //...
    zstatistic_fini(&g_stat_res, SRI_SIZE, hots);
}
#endif // if 0

#endif
