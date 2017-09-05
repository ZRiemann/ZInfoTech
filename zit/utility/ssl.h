#ifndef _ZUTILITY_SSL_H_
#define _ZUTILITY_SSL_H_
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
#include <zit/base/type.h>

ZC_BEGIN

ZAPI int zbase64(int is_encrypt, const char* in, int in_len, char* out, int* out_len);

ZC_END
/**@fn zbase64(int is_encode, const char* in, int in_len, char* out, int* out_len)
   @brief encrypt/decrypt <in> to <out> by base64
   @param int is_encode [in] 0:encrypt 1:decrypt
   @param const char* in [in] input data
   @param int in_len [in] intput data length
   @param char* out [out] output buffer
   @param int* out_len [in|out] in:output buffer size; out:decrypt data length.
 */

#endif
