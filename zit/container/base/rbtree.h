#ifndef _Z_CONT_RB_TREEX_H_
#define _Z_CONT_RB_TREEX_H_
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
 * @file zit/container/base/rbtree.h
 * @brief A classical red-black tree implements
 *
 * - see https://github.com/prasanthmadhavan/Red-Black-Tree.git
 * - extends
 *   -# thread safe;
 *   -# has memory allocate, support any type and structurs;
 *   -# support multiple items by one key;
 *   -# support hash compare, default key type is int32_t;
 */
#include <zit/base/type.h>
#include <zit/base/atomic.h>
#include <zit/container/base/alloc.h>
#include <zit/thread/spin.h>
ZC_BEGIN

enum rbtree_node_color { RED, BLACK };

typedef struct zbtree_node_s {
    struct zbtree_node_s *left;
    struct zbtree_node_s *right;
    struct zbtree_node_s *parent;
    struct zbtree_node_s *next; /* multiple list  */
    enum rbtree_node_color color;
    char data[];
}zbtnode_t;

typedef struct zbinary_tree_s{
    zbtnode_t *root;
    zalloc_t *alloc;
    zoperate cmp; /** 0-int32_t*(default) !0-zbtnode_t* */
    zspinlock_t spin; /** thread safe operate */
    int data_size; /** data size */
    int multiple; /** 1- multiple 0- unique(default) */
}zbtree_t;

#define zrbn_is_red(node) (node && node->color == RED)
#define zrbn_is_black(node) (!node || node->color == BLACK)
#define zrbn_key(node) *(int32_t*)node->data
#define zrbn_set_black(node) do{node->color = BLACK;}while(0)
#define zrbn_set_red(node) do{node->color = RED;}while(0)

ZAPI zerr_t zrbtree_create(zbtree_t **tree, int32_t node_size, int32_t capacity);
ZAPI zerr_t zrbtree_destroy(zbtree_t *tree);
ZAPI void zrbtree_foreach(zbtnode_t *root, int order, zoperate do_each, zvalue_t hint);
ZAPI void zrbtree_print(zbtree_t *tree);
ZAPI zerr_t zrbtree_verify(zbtree_t *tree);
ZAPI zerr_t zrbtree_insert(zbtree_t *tree, zbtnode_t *node);
ZAPI zerr_t zrbtree_erase(zbtree_t *tree, zbtnode_t *key);

zinline zerr_t zrbtree_compare(zbtree_t *tree, zbtnode_t *refer, zbtnode_t *key){
    return tree->cmp ? tree->cmp((zvalue_t)refer->data, NULL, (zvalue_t)key) : \
        (*(int32_t*)refer->data - *(int32_t*)key);
}

zinline zerr_t zrbtree_compare_node(zbtree_t *tree, zbtnode_t *refer, zbtnode_t *node){
    return tree->cmp ? tree->cmp((zvalue_t)refer->data, NULL, (zvalue_t)node->data) :\
        (*(int32_t*)refer->data - *(int32_t*)node->data);
}

zinline zerr_t zrbtree_get_node(zbtree_t *tree, zbtnode_t **node){
    return zalloc_pop(tree->alloc, (zvalue_t*)node);
}

zinline zerr_t zrbtree_push_node(zbtree_t *tree, zbtnode_t **node){
    return zalloc_push(tree->alloc, (zvalue_t*)node);
}

zinline zerr_t zrbtree_find(zbtree_t *tree, zbtnode_t *key, zbtnode_t **node){
    zbtnode_t *refer = tree->root;
    zerr_t ret = ZEQUAL;
    zspin_lock(&tree->spin);
    while(refer){
        ret = zrbtree_compare(tree, refer, key);
        if(0 < ret){
            refer = refer->left;
        }else if(0 > ret){
            refer = refer->right;
        }else{
            *node = refer;
            zspin_unlock(&tree->spin);
            return ZEOK;
        }
    }
    *node = NULL;
    zspin_unlock(&tree->spin);
    return ZENOT_EXIST;
}

ZC_END
#endif