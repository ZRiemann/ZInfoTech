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
#include <zit/utility/ssl.h>
#include "modp_b64/modp_b64.cc"

int zbase64(int is_encrypt, const char* in, int in_len, char* out, int* out_len){
  //  printf("dummy zbase64()...\n");
  return ZENOT_SUPPORT;
}
