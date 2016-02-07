/**@file trace.c
 * @brief trace message
 */
#include "export.h"
#include <zit/base/type.h>
#include <zit/base/trace.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef ZSYS_WINDOWS
#include <windows.h>
#include <lmerr.h>
#pragma warning(disable:4996)
#endif

#define ZTRACE_FLAG_DBG 0x01
#define ZTRACE_FLAG_MSG 0x02
#define ZTRACE_FLAG_WAR 0x04
#define ZTRACE_FLAG_ERR 0X08
#define ZTRACE_BUF_SIZE 4096
#define ZTRACE_BUF_SIZE1 4095

ztrace g_ztrace = NULL;
void* g_ztrace_user = NULL;
int g_ztrace_flag = 0x000000ff;

int ztrace_reg(ztrace fn, void* user) {
	g_ztrace = fn;
	g_ztrace_user = user;
	return ZEOK;
}

int ztrace_ctl(int flag) {
	g_ztrace_flag = (flag & 0x000000ff);
	return ZEOK;
}

int zdbg(const char* msg, ...) {
	if (g_ztrace && (g_ztrace_flag & ZTRACE_FLAG_DBG)) {
		va_list arglist;
		char buf[ZTRACE_BUF_SIZE];
		buf[ZTRACE_BUF_SIZE1] = 0;
		va_start(arglist, msg);
		vsnprintf(buf, ZTRACE_BUF_SIZE1, msg, arglist);
		va_end(arglist);
		g_ztrace(ZTRACE_LEVEL_DBG, g_ztrace_user, buf);
	}
	return ZEOK;
}
int zmsg(const char* msg, ...) {
	if (g_ztrace && (g_ztrace_flag & ZTRACE_FLAG_DBG)) {
		va_list arglist;
		char buf[ZTRACE_BUF_SIZE];
		buf[ZTRACE_BUF_SIZE1] = 0;
		va_start(arglist, msg);
		vsnprintf(buf, ZTRACE_BUF_SIZE1, msg, arglist);
		va_end(arglist);
		g_ztrace(ZTRACE_LEVEL_MSG, g_ztrace_user, buf);
	}
	return ZEOK;
}

int zwar(const char* msg, ...) {
	if (g_ztrace && (g_ztrace_flag & ZTRACE_FLAG_DBG)) {
		va_list arglist;
		char buf[ZTRACE_BUF_SIZE];
		buf[ZTRACE_BUF_SIZE1] = 0;
		va_start(arglist, msg);
		vsnprintf(buf, ZTRACE_BUF_SIZE1, msg, arglist);
		va_end(arglist);
		g_ztrace(ZTRACE_LEVEL_WAR, g_ztrace_user, buf);
	}
	return ZEOK;
}
int zerr(const char* msg, ...) {
	if (g_ztrace && (g_ztrace_flag & ZTRACE_FLAG_DBG)) {
		va_list arglist;
		char buf[ZTRACE_BUF_SIZE];
		buf[ZTRACE_BUF_SIZE1] = 0;
		va_start(arglist, msg);
		vsnprintf(buf, ZTRACE_BUF_SIZE1, msg, arglist);
		va_end(arglist);
		g_ztrace(ZTRACE_LEVEL_ERR, g_ztrace_user, buf);
	}
	return ZEOK;
}

#ifdef ZSYS_WINDOWS
static int zwin_lasterror(int code, char* buf, int buflen) {
	int ret = ZEFUN_FAIL;
	HMODULE hModule = NULL;
	LPSTR MessageBuffer;
	DWORD dwBufferLength;
	DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
	if (code >= NERR_BASE && code <= MAX_NERR) {
		hModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
		if (hModule != NULL)
			dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
	}

	if (dwBufferLength = FormatMessageA(dwFormatFlags, hModule, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&MessageBuffer, 0, NULL)) {
		MessageBuffer[dwBufferLength - 2] = 0;
		_snprintf(buf, buflen - 1, MessageBuffer);
		LocalFree(MessageBuffer);
		ret = ZEOK;
	}
	if (NULL != hModule) {
		FreeLibrary(hModule);
	}
	return ret;
}
#endif

const char*  zstrerr(int code) {
	const char* msg = "unknownd error";
	static char lasterr[ZTRACE_BUF_SIZE] = { 0 };
	//task delay zstrerr()
	if (((code & ZEMASK) == ZEMASK) || (ZEOK == code)) {
		switch (code) {
		case ZEOK:
			msg = "ZEOK"; break;
		case ZEFAIL:
			msg = "ZENormal error"; break;
		default:
			break;
		}
	}
	else {
		lasterr[ZTRACE_BUF_SIZE1] = 0;
#ifdef ZSYS_POSIX
		snprintf(lasterr, ZTRACE_BUF_SIZE1, strerror(code));
#else//ZSYS_WINDOWS
		zwin_lasterror(code, lasterr, ZTRACE_BUF_SIZE);
#endif
		msg = lasterr;
	}
	return msg;
}
