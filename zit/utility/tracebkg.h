#ifndef _ZTRACE_BKG_H_
#define _ZTRACE_BKG_H_
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
#include <zit/base/platform.h>
#include <zit/base/trace.h>
ZC_BEGIN

ZAPI int ztrace_bkgctl(ztrace cb);
ZAPI int ztrace_bkgend();
ZAPI int ztrace_bkg(int level, void *user, const char* msg);

ZAPI int ztrace_bkgbuf(char **buf, int len);
ZAPI int ztrace_0cpy_bkg(int len, void *user, const char* msg);

ZC_END

/** @fn int ztrace_logctl(const char* fname, int log_size)
 *  @brief control trace log file name, max size and *close log file*
 *  @param const char* fname [in] log file name
 *  @param int log_size [in] set log max size
 *  @note
 *   ztrace_logctl(NULL, 0); // if open then close log file
 */
/** @fn int trace_log(int level, void *user const char* msg);
 *  @brief write <msg> to log file
 *  @param int level [in] log level, defined in <zit/base/trace.h>
 *  @param void *user [in] user data
 *  @param const char * [in] log message
 */
#endif
