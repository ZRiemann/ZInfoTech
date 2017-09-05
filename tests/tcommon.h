#ifndef _ZTST_COMMON_H_
#define _ZTST_COMMON_H_

#include <zit/base/type.h>
#include <stdio.h>

zinline zerr_t print_iany(ZOP_ARG){
    int *i = (int*)hint;
    printf("%c%04d", (*i & 0xf) ? ' ' : '\n', ((zany_t*)in)->i);
    ++(*i);
    return 0;
}

#endif
