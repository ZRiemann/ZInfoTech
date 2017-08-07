#ifndef _Z_STATISTIC_H_
#define _Z_STATISTIC_H_
/**
 * @file zit/base/statistic.h
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
 *      |- memory
 *      \- module statistic 
 */
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <zit/thread/spin.h> ///< spin lock static
#include <time.h>

#define ZSTATIS_OTYPE_UNKNOW 0
#define ZSTATIS_OTYPE_QUEUE 1

/**
 * @brief A object function call counter and frequency recode
 */
typedef struct zstatistic_cnt{
    zatm64_t *total_cnt; ///< function count;
    time_t last_call; ///< last call time
    zatm64_t *interval_cnt[3]; ///< [0]:<1 sec [2]:1~10 sec [3]:>10 sec
}zstat_cnt_t;
/**
 * @brief touch a statistic counter
 * @param [in] cnt a zstat_cnt_t pointer to counter.
 */
void zstat_cnt_touch(zstat_cnt_t *cnt);

inline void zstat_cnt_touch(zstat_cnt_t *cnt){
    time_t now;
    int interval;
    time(&now);
    zatm_inc(cnt->total_cnt);
    interval = now - cnt->last_call;
    interval = interval < 1 ? 0 : ((interval >=1 && interval <= 10) ? 1 : 2);
    zatm_inc(cnt->interval_cnt[interval]);
    cnt->last_call = now;
}

// about statistic identify (for root list)
typedef struct zstatistic_identity_s{
    zotype_t type; ///< statistic type, for free by other(root) module
    char name[64]; ///< statistic readable name
    time_t create_time; ///< the queue create time
    time_t destroy_time; ///< the queue destroy time
    zoperate to_string; ///< convert statistic to readable string.
    zoperate release; ///< release statistic infomation;
    char info[]; ///< statistic informtion
}zstat_id_t;

typedef struct zstatistic_s{
    int flag; ///< 1:enable statistic; 0:disable
    zoperate cb; ///< queue call back
    zstat_id_t *stat; ///< statistic identity
}zstatistic_t;


#endif
