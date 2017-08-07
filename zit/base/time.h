#ifndef _ZBASE_TIME_H_
#define _ZBASE_TIME_H_
/** @file zit/base/time.h
 *   @note
 *   @brief
 *   @author  ZRiemann
 *   @date   2016-02-03
 *   @note
 */

#include <zit/base/type.h>
#include <time.h>

ZC_BEGIN

ZAPI char* zstr_time(time_t* tm, char* buf);
ZAPI char* zstr_now(char* buf);
ZAPI time_t ztm2time(struct tm* ptm);
ZAPI void ztime2tm(const time_t* time, struct tm* ptm);
ZAPI time_t zstr2time(const char* strTime);
ZAPI void zconvert_time(uint64_t* time, int flag);

typedef struct _ztime_t{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
    int microsecond;
    int nanosecond;

    int day_of_week;
}ztime_t;

typedef enum
{
    ZTP_SEC = 1,
    ZTP_MILLISEC = 2,
    ZTP_MICROSEC = 3,
    ZTP_NANOSEC = 4
}ztime_precision_t;

ZAPI int zget_localtime(ztime_t* ptm);
ZAPI char* zstr_systime(ztime_t* ptm, char* buf, ztime_precision_t precision);
ZAPI char* zstr_systime_now(char* buf, ztime_precision_t precision);

#if 0   // high performance clock
typedef struct _zinterval
{
    uint64_t second;
    uint64_t nanoSecond;
}zinterval_t;
#endif


// Sleep function
ZAPI void zsleepsec(int sec);
ZAPI void zsleepms(int ms);
ZAPI void zsleepus(int us);

// time interval count
typedef void *ztick_t;
ZAPI ztick_t ztick();
ZAPI zerr_t ztock(ztick_t handle, int *sec, int *usec);

ZC_END


/**
 * @fn char* zstr_time(time_t* tm, char* buf);
 * @brief convert tm to local string trime "yy-mm-dd hh:mm:ss"
 * @param [in] time_t* tm
 * @param [out] char* buf
 * @return return pointer to buf
 * @note suggest buf size >= 32
 */
/**
 * @fn char* zstr_now(char* buf);
 * @brief "yy-mm-dd hh:mm:ss" format local time string
 * @param [out] buf time string buffer
 * @return buf
 * @note suggest buf size >= 32
 */
/**
 * @fn time_t ztm2time(struct tm* ptm)
 * @brief  convert tm to time_t
 * @param [in] ptm pointer to struct tm;
 * @return time_t
 */
/**
 * @fn void ztime2tm(struct tm* ptm,const time_t* time)
 * @brief convert time_t to  tm
 * @param [out] ptm
 * @param [in] time
 * @return void
 */
/**
 * @fn time_t zstr2time(const char* strTime)
 * @brief convert "yy-mm-dd hh:mm:ss" to time_t
 * @param [in] local time string
 * @return time_t
 */
/**
 * @fn void zconvert_time(unsigned __int64* time, int flag)
 * @brief system / utc time convert
 * @param [in,out] unsigned __int64* time 
 * @param [in] flag: 0 - sys2utc 1 - utc2sys
 * @return ZEOK	
 */

#endif
