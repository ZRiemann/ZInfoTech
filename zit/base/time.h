#ifndef _ZBASE_TIME_H_
#define _ZBASE_TIME_H_

#include <zit/base/type.h>
#include <time.h>

#ifdef __cplusplus
extern "C"{
#endif

ZEXP char* zstr_time(time_t* tm, char* buf);
ZEXP char* zstr_now(char* buf);
ZEXP time_t ztm2time(struct tm* ptm);
ZEXP void ztime2tm(const time_t* time, struct tm* ptm);
ZEXP time_t zstr2time(const char* strTime);
ZEXP void zconvert_time(uint64_t* time, int flag);

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

ZEXP int zget_localtime(ztime_t* ptm);
ZEXP char* zstr_systime(ztime_t* ptm, char* buf, ztime_precision_t precision);
ZEXP char* zstr_systime_now(char* buf, ztime_precision_t precision);

#if 0   // high performance clock
typedef struct _zinterval
{
    uint64_t second;
    uint64_t nanoSecond;
}zinterval_t;
#endif


// Sleep function
ZEXP void zsleepsec(int sec);
ZEXP void zsleepms(int ms);
ZEXP void zsleepus(int us);


#ifdef __cplusplus
}
#endif



/** @file zit/base/time.h
*   @note 
*   @brief   
*   @author  ZRiemann
*   @date   2016-02-03

*   @note	
*/
/** @fn		char* zstr_time(time_t* tm, char* buf);
*   @brief 	将tm转换为 本地可读时间格式 "yy-mm-dd hh:mm:ss"
*   @param	time_t* tm [IN]
*   @param  char* buf [OUT] 
*	@return	传入的字符串指针
*   @note   建议buf长度32
*/
/** @fn		char* zstr_now(char* buf);
*   @brief 	本地可读时间格式 "yy-mm-dd hh:mm:ss" 当前时间
*   @param	char* buf [OUT]
*	@return	传入额字符串指针
*   @note   建议buf长度32
*/
/** @fn		time_t ztm2time(struct tm* ptm)
*   @brief  将tm 转换为 time_t
*   @param	struct tm* ptm [IN]
*	@return time_t 值
*/
/** @fn		void ztime2tm(struct tm* ptm,const time_t* time)
*   @brief 	将time_t 转换为 tm
*   @param	struct tm* ptm [OUT]
*   @param  const time_t* time [IN]
*	@return	
*/
/** @fn		time_t zstr2time(const char* strTime)
*   @brief 	将本地可读时间格式 "yy-mm-dd hh:mm:ss" 转为 time_t
*   @param	const char* strTime [IN] 本地可读时间
*	@return	time_t 值
*/
/** @fn		void zconvert_time(unsigned __int64* time, int flag)
*   @brief 	system / utc time convert
*   @param	unsigned __int64* time [IN|OUT]
*   @param  int flag [IN] flag: 0 - sys2utc 1 - utc2sys
*	@return ZEOK	
*/

#endif
