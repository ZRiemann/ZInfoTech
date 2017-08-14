#ifndef _ZBASE_TIME_H_
#define _ZBASE_TIME_H_
/** @file zit/base/time.h
 *   @note
 *   @brief
 *   @author  ZRiemann
 *   @date   2016-02-03
 *   @note
 */
#include <zit/base/platform.h>
#include <zit/base/type.h>
#include <time.h>

ZC_BEGIN

ZAPI char* zstr_time(time_t* tm, char* buf);
ZAPI char* zstr_now(char* buf);
ZAPI time_t ztm2time(struct tm* ptm);
ZAPI void ztime2tm(const time_t* time, struct tm* ptm);
ZAPI time_t zstr2time(const char* strTime);
ZAPI void zconvert_time(uint64_t* time, int flag);

typedef struct ztime_s{
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

// Sleep function
ZAPI void zsleepsec(int sec);
ZAPI void zsleepms(int ms);
ZAPI void zsleepus(int us);

// time interval count
typedef void *ztick_t;
ZAPI ztick_t ztick();
ZAPI zerr_t ztock(ztick_t handle, int *sec, int *usec);

ZC_END

// Acquiring high-resolution time stamps
#ifdef ZSYS_POSIX
#include <sys/time.h>
#include <unistd.h>
typedef struct timeval ztime_stamp_t;

#define ztime_stamp(ts) gettimeofday(ts, NULL)
#define ztime_interval(begin, end, interval) do{\
    int sec;\
    (begin)->tv_usec %= 1000000;\
    (end)->tv_usec %= 1000000;\
    if((end)->tv_usec < (begin)->tv_usec){\
        sec = (end)->tv_sec - (begin)->tv_sec - 1;\
        *(interval) = (end)->tv_usec + 1000000 - (begin)->tv_usec;\
    }else{\
        sec = (end)->tv_sec - (begin)->tv_sec;\
        *(interval) = (end)->tv_usec - (begin)->tv_usec;\
    }\
    *(interval) += (sec * 1000000);\
    }while(0)
#define ztime_sub(begin, end) ((end)->tv_usec < (begin)->tv_usec ? ((end)->tv_sec -=((begin)->tv_sec+1), (end)->tv_usec += (1000000 - (begin)->tv_usec)) : ((end)->tv_sec -= (begin)->tv_sec, (end)->tv_usec -= (begin)->tv_usec))
#define ztime_sub2us(us, sub) *us = ((sub)->tv_sec * 1000000) + (sub)->tv_usec
#define ztime_ts2str(ts, buf) sprintf(buf, "%lu.%06lu", (ts)->tv_sec, (ts)->tv_usec)
#define ztime_str2ts(ts, buf) do{\
char *pc = NULL;\
pc = strstr(buf, ".");\
*pc = 0;\
(ts)->sec = atoi(buf);\
pc++;\
(ts)->usec = atoi(pc);\
*(--pc) = '.';\
}while(0)

#else
#include <Windows.h>
#include <time.h>
typedef LARGE_INTEGER ztime_stamp_t;

#define ztime_stamp(ts) (QueryPerformanceCounter(ts) - 1)
#define ztime_interval(begin, end, interval) do{    \
    ztime_stamp_t frequency;\
    QueryPerformanceFrequency(&frequency);\
    *(interval) =  (((end)->QuadPart - (begin)->QuadPart) * 1000000) / frequency.QuadPart;\
    }while(0)
#define ztime_sub(begin, end) ((end)->QuadPart -= (begin)->QuadPart)
#define ztime_sub2us (us, sub) do{\
        ztime_stamp_t frequency;                \
        QueryPerformanceFrequency(&frequency);                          \
        *us =  ((sub)->QuadPart * 1000000) / frequency.QuadPart; \
    }while(0)
#define ztime_ts2str(ts,buf) sprintf(buf, "%llu", (long long unsigned int) (ts)->QuadPart)
#define ztime_str2ts(ts, buf) (ts)->QuadPart = _atoi64()
#endif



//zinline zerr_t ztime_stamp(ztime_stamp_t *ts);
//zinline void ztime_interval(ztime_stamp_t *begin, ztime_stamp_t *(end), uint64_t *interval);

/**
 * @fn zerr_t time_stamp(ztime_stamp_t *ts)
 * @brief Retrieves the current value a high resolution (<=1us) time stamp
 * @param [out] ts pointer to time stamp
 * @return ZEOK/ZEFAIL
 */
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
