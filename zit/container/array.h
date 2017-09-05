#ifndef _ZCONTAINER_ARRAY_H_
#define _ZCONTAINER_ARRAY_H_
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
/**
 * @file zit/container/array.h
 * @brief A dichotomy search operate sorted array. no lock operate.
 *
 * - Performance test case (set ZARR_CHECK 0)
 *   zit.array:
 *   $ make/release/zit_test array 1
 *   size     find(0~size-1)(sec) max_erase_1  insert_min
 *   5000     0.001189            0.000009
 *            0.001153            0.000009
 *
 *   50000    0.004848            0.000032
 *            0.006625            0.000043
 *
 *   500000   0.037476            0.000654
 *            0.036678            0.000773
 *
 *   5000000  0.389212            0.008738
 *            0.388266            0.008259     0.303830
 *
 *   std::set:
 *   $ make/release/zpp_test array 1
 *   size     find                total_erase  insert_total
 *   5000     0.000423            0.000278
 *            0.000399            0.000288     0.000474
 *
 *   50000    0.006090            0.003156
 *            0.005788            0.003510     0.009962
 *
 *   500000   0.100725            0.073833
 *            0.100132            0.073953     0.155410
 *
 *   5000000  1.296026            1.034311
 *            1.305482            1.040404     2.250305
 */
#include <zit/base/type.h>
ZC_BEGIN
/**
 * @brief A ascending sorted array structure
 */
typedef struct zarray_any_s{
    zany_t *array; /** Array buffer */
    uint32_t size; /** Array size */
    uint32_t capacity; /** Array capacity */
}zarray_t;

#if 0
zerr_t zarray_create(zarray_t **arr, int capacity);
zerr_t zarray_destroy(zarray_t *arr);
zerr_t zarray_find(zarray_t *arr, zany_t val, int *pos);
zerr_t zarray_findx(zarray_t *arr, zoperate compare, zvalue_t hint, int *pos); // task dellay
zerr_t zarray_insert(zarray_t *arr, zany_t value);
zerr_t zarray_erase(zarray_t *arr, zany_t value);
zerr_t zarray_foreach(zarray_t *arr, zoperate op, zvalue_t hint);
#endif

/*
 * inline definitions
 */
#include <zit/base/error.h>
#include <stdlib.h>
#include <string.h>

zinline zerr_t zarray_create(zarray_t **arr, int capacity){
    int ret = ZEOK;
    zarray_t *sa = (zarray_t*)calloc(1, sizeof(zarray_t));
    zany_t *array = (zany_t*)calloc(capacity, sizeof(zany_t));
    if(sa && array){
        sa->array = array;
        sa->capacity = capacity;
        sa->size = 0;
        *arr = sa;
    }else{
        free(sa);
        free(array);
        ret = ZMEM_INSUFFICIENT;
    }
    return ret;
}

zinline zerr_t zarray_destroy(zarray_t *arr){
    if(arr){
        free(arr->array);
        free(arr);
    }
    return ZEOK;
}

zinline zerr_t zarray_find(zarray_t *arr, zany_t val, int *pos_){
    int begin, end, pos;//, size1;
    zerr_t ret;
    begin = 0;
    end = arr->size - 1;
    pos = (arr->size) >> 1;
    ret = ZNOT_EXIST;

    while(end >= 0){
        if(val.i64 == arr->array[pos].i64){
            ret = ZEQUAL;
            break;
        }
        if(val.i64 > arr->array[pos].i64){
            begin = pos;
            ret = ZGREAT;
            pos = (begin + end + 1) >> 1;
        }else{
            end = pos;
            ret = ZLITTLE;
            pos = (begin + end) >> 1;
        }
        if(begin == end){
            break;
        }
    }
    *pos_ = pos;
    return ret;
}

zinline zerr_t zarray_insert(zarray_t *arr, zany_t value){
    /* Assert param　*/
    int pos = 0;
    zerr_t ret = ZEOK;
    /* Reallocate memory if <arr> size equal to capacity. */
    if(arr->size+1 == arr->capacity){
        uint32_t append = 4096;
        zany_t *array = NULL;
        append = (arr->capacity > append) ? (append + arr->capacity) : (arr->capacity << 1);
        array = (zany_t*)realloc(arr->array, append * sizeof(zany_t));
        if(array){
            arr->array = array;
            arr->capacity = append;
        }else{
            return ZMEM_INSUFFICIENT;
        }
    }
    /* Insert <value> to <arr->array> by ascending order */
    ret = zarray_find(arr, value, &pos);
    switch(ret){
    case ZGREAT:
        ++pos;
    case ZLITTLE:
        memmove(&arr->array[pos+1], &arr->array[pos], (arr->size - pos)*sizeof(zany_t));
    case ZENOT_EXIST:
        arr->array[pos] = value;
        ++arr->size;
        ret = ZEOK;
        break;
    case ZEQUAL:
        /* Do nothing, unique insert item */
        ret = ZPARAM_INVALID;
        break;
    default:
        return ZENOT_SUPPORT;
    }
    return ret;
}

zinline zerr_t zarray_erase(zarray_t *arr, zany_t value){
    int pos = 0;
    int ret = 0;
    if(ZEQUAL == (ret = zarray_find(arr, value,  &pos))){
        --arr->size;
        memmove(&arr->array[pos], &arr->array[pos+1], (arr->size - pos)*sizeof(zany_t));
        zprint("erase[%d]<value:%d, size:%d>\n", pos, value.i, arr->size);
    }else{
        zprint("erase: %d ret: %d, pos: %d", value.i, ret, pos );
    }
    return ret;
}

zinline zerr_t zarray_foreach(zarray_t *arr, zoperate op, zvalue_t hint){
    for(uint32_t i=0; i<arr->size; ++i){
        op((zvalue_t)&arr->array[i], NULL, hint);
    }
    return ZEOK;
}

ZC_END

#endif
