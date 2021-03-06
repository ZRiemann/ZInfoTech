/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
#include "export.h"
#include <zit/base/error.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>

#define ZMS_PER_SEC 1000
#define ZUS_PER_SEC 1000000

#ifdef ZSYS_POSIX
#include <sys/time.h>
#include <unistd.h>
#else
#include <Windows.h>
#include <time.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* zstr_time(time_t* tm, char* buf){
	struct tm* tmnow = localtime(tm);
	if (NULL != buf){
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);
	}
	return buf;
}

char* zstr_now(char* buf){
	time_t now;
	struct tm* tmnow;
	time(&now);
	tmnow = localtime(&now);
	sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", tmnow->tm_year + 1900, tmnow->tm_mon + 1, tmnow->tm_mday, tmnow->tm_hour, tmnow->tm_min, tmnow->tm_sec);
	return buf;
}
time_t ztm2time(struct tm* ptm){
	ptm->tm_year -= 1900;
	ptm->tm_mon -= 1;
	ptm->tm_isdst = -1;
	return mktime(ptm);
}
void ztime2tm(const time_t* time, struct tm* ptm){
	struct tm* ptime;
	if (NULL == time || NULL == ptm){
		return;
	}
	ptime = localtime(time);
	ptm->tm_year = ptime->tm_year + 1900;
	ptm->tm_mon = ptm->tm_mon + 1;
	ptm->tm_mday = ptime->tm_mday;
	ptm->tm_hour = ptime->tm_hour;
	ptm->tm_min = ptime->tm_min;
	ptm->tm_sec = ptime->tm_sec;
}

time_t zstr2time(const char* strTime){
	struct tm t;
	// make tm...
	char* szEnd;
	char* szBegin;
	char szTime[32];
	//%d-%d-%d %d:%d:%d
	sprintf(szTime, "%s", strTime);

	szBegin = szTime;
	szEnd = strstr(szBegin, "-");
	*szEnd = 0;
	t.tm_year = atoi(szBegin);

	++szEnd;
	szBegin = szEnd;
	szEnd = strstr(szBegin, "-");
	*szEnd = 0;
	t.tm_mon = atoi(szBegin);

	++szEnd;
	szBegin = szEnd;
	szEnd = strstr(szBegin, " ");
	*szEnd = 0;
	t.tm_mday = atoi(szBegin);

	++szEnd;
	szBegin = szEnd;
	szEnd = strstr(szBegin, ":");
	*szEnd = 0;
	t.tm_hour = atoi(szBegin);

	++szEnd;
	szBegin = szEnd;
	szEnd = strstr(szBegin, ":");
	*szEnd = 0;
	t.tm_min = atoi(szBegin);

	++szEnd;
	szBegin = szEnd;
	t.tm_sec = atoi(szBegin);

	return ztm2time(&t);
}

void zconvert_time(uint64_t* time, int flag){
	uint64_t jan1970ft = 0;//(uint64_t)116444736000000000;
	if (flag){
		*time = (*time) * 10000000 + jan1970ft;
	}
	else{
		struct tm* tmnow;
		*time = ((*time) - jan1970ft) / 10000000;
		tmnow = gmtime((time_t*)time);
		*time = mktime(tmnow);
	}
}

int zget_localtime(ztime_t* ptm){
	int ret;
#ifdef ZSYS_POSIX
	struct timeval tv;
	struct tm now;
	ret = gettimeofday(&tv,NULL);
	if( ZEOK != ret ){
		ret = ZEFAIL;
		return ret;
	}
	localtime_r(&tv.tv_sec, &now);

	ptm->year = now.tm_year + 1900;
	ptm->month = now.tm_mon + 1;
	ptm->day = now.tm_mday;
	ptm->hour = now.tm_hour;
	ptm->minute = now.tm_min;
	ptm->second = now.tm_sec;
	ptm->millisecond = tv.tv_usec/1000;
	ptm->microsecond = tv.tv_usec%1000;
	ptm->nanosecond = 0;
	ptm->day_of_week = now.tm_wday;
#else // ZSYS_WINDOWS
	SYSTEMTIME st;
	ret = ZEOK;
	GetLocalTime(&st);

	ptm->year = st.wYear;
	ptm->month = st.wMonth;
	ptm->day = st.wDay;
	ptm->hour = st.wHour;
	ptm->minute = st.wMinute;
	ptm->second = st.wSecond;
	ptm->millisecond = st.wMilliseconds;
	ptm->microsecond = 0;
	ptm->nanosecond = 0;
	ptm->day_of_week = st.wDayOfWeek;
#endif
	return ret;
}
char* zstr_systime(ztime_t* ptm, char* buf, ztime_precision_t precision){
	switch (precision){
	case ZTP_SEC:
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", ptm->year, ptm->month, \
			ptm->day, ptm->hour, ptm->minute, ptm->second);
		break;
	case ZTP_MICROSEC:
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d.%03d%03d", ptm->year, ptm->month, ptm->day, \
			ptm->hour, ptm->minute, ptm->second, ptm->millisecond, ptm->microsecond);
		break;
	default:
		sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d.%03d", ptm->year, ptm->month, ptm->day, \
			ptm->hour, ptm->minute, ptm->second, ptm->millisecond);
		break;
	}
	return buf;
}
char* zstr_systime_now(char* buf, ztime_precision_t precision){
	ztime_t ztm;
	zget_localtime(&ztm);
	return zstr_systime(&ztm, buf, precision);
}


void zsleepsec(int sec){
#ifdef ZSYS_POSIX
	sleep(sec);
#else //ZSYS_WINDOWS
	Sleep(sec * 1000);
#endif
}
void zsleepms(int ms){
#ifdef ZSYS_POSIX
	usleep(ms*1000);
#else//ZSYS_WINDOWS
	Sleep(ms);
#endif
}
void zsleepus(int us){
#ifdef ZSYS_POSIX
	usleep(us);
#else//ZSYS_WINDOWS
	Sleep(us / 1000);
#endif
}

void *ztick(){
#ifdef ZSYS_POSIX
	int ret;
	struct timeval *tv;
	tv = (struct timeval*)malloc(sizeof(struct timeval));
	if(tv){
		ret = gettimeofday(tv,NULL);
		if( 0 != ret ){
			free(tv);
			tv = NULL;
		}
	}
#else
	LARGE_INTEGER* tv;
	tv = (LARGE_INTEGER*)calloc(1, sizeof(LARGE_INTEGER));
	if (tv){
		QueryPerformanceCounter(tv); // window xp or later always success
	}
#endif
	return tv;
}

zerr_t ztock(void *handle, int *_sec, int *_usec){
#ifdef ZSYS_POSIX
	struct timeval *tv;
	struct timeval tvend;
	int sec;
	int usec;
	struct tm now;
	ztime_t begin;
	ztime_t end;
	ztime_t *ptm;

	if(!handle){
		return ZPARAM_INVALID;
	}
	gettimeofday(&tvend,NULL);
	tv = (struct timeval*)handle;

	if(tv){
		tvend.tv_usec %= 1000000;
		tv->tv_usec %= 1000000;

		ptm = &begin;
		localtime_r(&tv->tv_sec, &now);

		ptm->year = now.tm_year + 1900;
		ptm->month = now.tm_mon + 1;
		ptm->day = now.tm_mday;
		ptm->hour = now.tm_hour;
		ptm->minute = now.tm_min;
		ptm->second = now.tm_sec;
		ptm->millisecond = tv->tv_usec/1000;
		ptm->microsecond = tv->tv_usec%1000;
		ptm->nanosecond = 0;
		ptm->day_of_week = now.tm_wday;

		ptm = &end;
		localtime_r(&tvend.tv_sec, &now);

		ptm->year = now.tm_year + 1900;
		ptm->month = now.tm_mon + 1;
		ptm->day = now.tm_mday;
		ptm->hour = now.tm_hour;
		ptm->minute = now.tm_min;
		ptm->second = now.tm_sec;
		ptm->millisecond = tvend.tv_usec/1000;
		ptm->microsecond = tvend.tv_usec%1000;
		ptm->nanosecond = 0;
		ptm->day_of_week = now.tm_wday;


		sec = (int)(tvend.tv_sec - tv->tv_sec);
		if(tvend.tv_usec < tv->tv_usec){
			--sec;
			tvend.tv_usec += 1000000;
		}
		usec = tvend.tv_usec - tv->tv_usec;
		zdbg("\nztick:%d:%d:%d.%03d%03d\nztock:%d:%d:%d.%03d%03d\ninterval:%d.%06d"
			"\nztick:%llu.%06lu\nztock:%llu.%06lu",
			begin.hour, begin.minute, begin.second, begin.millisecond, begin.microsecond,end.hour, end.minute, end.second, end.millisecond, end.microsecond,sec,usec,
			tv->tv_sec, tv->tv_usec, tvend.tv_sec, tvend.tv_usec);
	}else{
		usec = 0;
		sec = 0;
	}
	if(_sec){
		*_sec = sec;
	}
	if(_usec){
		*_usec = usec;
	}
    free(handle);
	return ZOK;
#else
	LARGE_INTEGER *begin = (LARGE_INTEGER*)handle;
	LARGE_INTEGER end, escaped, frequency;
	int sec, usec;
	if (!handle){
		return ZPARAM_INVALID;
	}
	QueryPerformanceCounter(&end);

	QueryPerformanceFrequency(&frequency);
	escaped.QuadPart = ((end.QuadPart - begin->QuadPart) * ZUS_PER_SEC) / frequency.QuadPart; // micro second
	sec = escaped.QuadPart / ZUS_PER_SEC;
	usec = escaped.QuadPart % ZUS_PER_SEC;
	zdbg("interval: %d.%06d", sec, usec);
	if (_sec){
		*_sec = sec;
	}
	if (_usec){
		*_usec = usec;
	}
    free(handle);
	return ZOK;
#endif
}