#ifndef _ZUTILITY_SSL_H_
#define _ZUTILITY_SSL_H_

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
