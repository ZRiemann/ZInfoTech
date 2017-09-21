#ifndef _ZCONTAINER_MAP_H_
#define _ZCONTAINER_MAP_H_
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
 * @file A key value map
 *
 * - zit.map VS std::map
 *   $ make/release/zpp_test map 1
 *   zit.map:  (tail)              (max 1)
 *   size      insert    find      erase
 *   5000      0.000830  0.000466
 *             0.000563  0.000142  0.000005
 *   50000     0.009735  0.002936
 *             0.003622  0.001968  0.000070
 *   500000    0.045397  0.019126  0.001398
 *             0.040593  0.021185  0.003251
 *   5000000   0.403836  0.237986  0.018181
 *             0.442166  0.239925  0.016174
 *   std::map: (total)   (find)    (total)
 *   5000      0.000583  0.000427  0.000269
 *             0.000478  0.000418  0.000266
 *   50000     0.009049  0.012715  0.003399
 *             0.009457  0.005837  0.003258
 *   500000    0.151104  0.100385  0.074132
 *             0.149304  0.095627  0.075379
 *   5000000   4.707265  1.285161  1.032917
 *             2.312755  1.262648  1.049983
 */
#include <zit/base/type.h>

ZC_BEGIN

typedef struct zmap_s{
    zpair_t *pair; /** pair array */
    uint32_t size; /** map size*/
    uint32_t capacity; /** map capacity*/
}zmap_t;

#if 0
zinline zerr_t zmap_create(zmap_t **map, int capacity);
zinline zerr_t zmap_destroy(zmap_t *map);
zinline zerr_t zmap_find(zmap_t *map, zany_t *key, zany_t **value); // key in:key out:pos
zinline zerr_t zmap_insert(zmap_t *map, zany_t key, zany_t value);
zinline zerr_t zmap_erase(zmap_t *map, zany_t key);
zinline zerr_t zmap_foreach(zmap_t *map, zoperate op, zvalue_t hint);
#endif

/*
 * inline definitions
 */
#include <zit/base/error.h>
#include <stdlib.h>
#include <string.h>

zinline zerr_t zmap_create(zmap_t **map, int capacity){
    int ret = ZEOK;
    zmap_t *mp = (zmap_t*)calloc(1, sizeof(zmap_t));
    zpair_t *pair = (zpair_t*)calloc(capacity, sizeof(zpair_t));
    if(mp && pair){
        mp->pair = pair;
        mp->capacity = capacity;
        mp->size = 0;
        *map = mp;
    }else{
        free(mp);
        free(pair);
        ret = ZMEM_INSUFFICIENT;
    }
    return ret;
}

zinline zerr_t zmap_destroy(zmap_t *map){
    if(map){
        free(map->pair);
        free(map);
    }
    return ZEOK;
}

zinline zerr_t zmap_find(zmap_t *map, zany_t key, zpair_t **pair, int *pos_){
    int begin, end, pos;//, size1;
    zerr_t ret;
    begin = 0;
    end = map->size - 1;
    pos = (map->size) >> 1;
    ret = ZNOT_EXIST;

    while(end >= 0){
        if(key.i64 == map->pair[pos].key.i64){
            ret = ZEQUAL;
            break;
        }
        if(key.i64 > map->pair[pos].key.i64){
            begin = pos;
            pos = (begin+end) >> 1;
            ret = ZGREAT;
        }else{
            end = pos;
            pos = (begin + end) >> 1;
            ret = ZLITTLE;
        }
        if((begin+1) >= end){
            ret = key.i64 - map->pair[begin].key.i64;
            if(ret == 0){
                ret = ZEQUAL;
                break;
            }else if(ret < 0){
                ret = ZLITTLE;
                break;
            }
            ret = key.i64 - map->pair[end].key.i64;
            if(0 == ret){
                ret = ZEQUAL;
                pos = end;
            }else if(ret < 0){
                ret = ZGREAT;
            }else{
                ret = ZGREAT;
                pos = end;
            }
            break;
        }
    }
    if(pos_){
        *pos_ = pos;
    }
    *pair = &map->pair[pos];
    return ret;
}

zinline zerr_t zmap_insert(zmap_t *map, zany_t key, zany_t value){
    /* Assert param　*/
    zerr_t ret = ZEOK;
    zpair_t* ppair = NULL;
    int pos = 0;
    /* Reallocate memory if <map> size equal to capacity. */
    if(map->size+1 == map->capacity){
        uint32_t append = 4096;
        zpair_t *array = NULL;
        append = (map->capacity > append) ? (append + map->capacity) : (map->capacity << 1);
        array = (zpair_t*)realloc(map->pair, append * sizeof(zpair_t));
        if(array){
            map->pair = array;
            map->capacity = append;
        }else{
            return ZEMEM_INSUFFICIENT;
        }
    }
    /* Insert <value> to <map->pair> by ascending order */
    ret = zmap_find(map, key, &ppair, &pos);
    switch(ret){
    case ZGREAT:
        ++pos;
    case ZLITTLE:
        memmove(&map->pair[pos+1], &map->pair[pos], (map->size - pos)*sizeof(zpair_t));
    case ZENOT_EXIST:
        map->pair[pos].key = key;
        map->pair[pos].value = value;
        ++map->size;
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

zinline zerr_t zmap_insert_pair(zmap_t *map, zpair_t* pair){
    return zmap_insert(map, pair->key, pair->value);
}

zinline zerr_t zmap_erase(zmap_t *map, zany_t key, zany_t *value){
    int ret = 0;
    zpair_t *pair = NULL;
    int pos = 0;

    if(ZEQUAL == (ret = zmap_find(map, key,  &pair, &pos))){
        *value = pair->value;
        --map->size;
        memmove(&map->pair[pos], &map->pair[pos+1], (map->size - pos)*sizeof(zpair_t));
        zprint("erase[%d]<value:%d, size:%d>\n", key.i, value->i, map->size);
    }else{
        zprint("erase: %d ret: %d, pos: %d", value->i, ret, key.i );
    }
    return ret;
}

zinline zerr_t zmap_foreach(zmap_t *map, zoperate op, zvalue_t hint){
    uint32_t i;
    for(i=0; i<map->size; ++i){
        op((zvalue_t)&(map->pair[i].value), NULL, hint);
    }
    return ZEOK;
}

ZC_END

#endif
