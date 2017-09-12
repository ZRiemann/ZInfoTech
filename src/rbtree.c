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
/* Remember the Properties
1. Each node is either red or black:
2. The root node is black.
3. All leaves (shown as NIL in the above diagram) are black and contain no data. Since we represent these empty leaves using NULL, this property is implicitly assured by always treating NULL as black. To this end we create a node_color() helper function:
4. Every red node has two children, and both are black (or equivalently, the parent of every red node is black).
5. All paths from any given node to its leaf nodes contain ithe same number of black node.
.... IDEA - traversing the tree, incrementing a black node count as i go. The first time i reach a leaf we save the count. Pinne adutha thavana leaves ethumbol compare chey...
Properties 4 and 5 together guarantee that no path in the tree is more than about twice as long as any other path, which guarantees that it has O(log n) height.
*/
#include "export.h"
#include <zit/container/base/rbtree.h>
#include <zit/base/atomic.h>
#include <zit/thread/spin.h>
#include <zit/base/trace.h>
#include <assert.h>
#include <stdlib.h>

zinline enum rbtree_node_color node_color(zbtnode_t *n){
    return n ? n->color : BLACK;
}
zinline zbtnode_t* grandparent(zbtnode_t* n) {
    assert (n != NULL);
    assert (n->parent != NULL);
    assert (n->parent->parent != NULL);
    return n->parent->parent;
}

zinline zbtnode_t* sibling(zbtnode_t* n) {
    assert (n != NULL);
    assert (n->parent != NULL);
    if (n == n->parent->left)
        return n->parent->right;
    else
        return n->parent->left;
}

zinline zbtnode_t* uncle(zbtnode_t* n) {
    assert (n != NULL);
    assert (n->parent != NULL);
    assert (n->parent->parent != NULL);
    return sibling(n->parent);
}

/*
zinline zbtnode_t* lookup_node(zbtree_t *t, zbtnode_t *key) {
    zbtnode_t* n = NULL;
    zspin_lock(&t->spin);
    n = t->root;
    while (n != NULL) {
        int comp_result = zrbtree_compare_node(t, n, key);
        if (comp_result == 0) {
            zspin_unlock(&t->spin);
            return n;
        } else if (comp_result > 0) {
            n = n->left;
        } else {
            assert(comp_result < 0);
            n = n->right;
        }
    }
    zspin_unlock(&t->spin);
    return n;
}
*/
zinline void replace_node(zbtree_t *t, zbtnode_t* oldn, zbtnode_t* newn) {
    if (oldn->parent == NULL) {
        t->root = newn;
    } else {
        if (oldn == oldn->parent->left)
            oldn->parent->left = newn;
        else
            oldn->parent->right = newn;
    }
    if (newn != NULL) {
        newn->parent = oldn->parent;
    }
}

zinline void rotate_left(zbtree_t *t, zbtnode_t* n) {
    zbtnode_t* r = n->right;
    replace_node(t, n, r);
    n->right = r->left;
    if (r->left != NULL) {
        r->left->parent = n;
    }
    r->left = n;
    n->parent = r;
}

zinline void rotate_right(zbtree_t *t, zbtnode_t* n) {
    zbtnode_t* L = n->left;
    replace_node(t, n, L);
    n->left = L->right;
    if (L->right != NULL) {
        L->right->parent = n;
    }
    L->right = n;
    n->parent = L;
}
static void insert_case1(zbtree_t *t, zbtnode_t* n);
zinline void insert_case5(zbtree_t *t, zbtnode_t* n) {
    n->parent->color = BLACK;
    grandparent(n)->color = RED;
    if (n == n->parent->left && n->parent == grandparent(n)->left) {
        rotate_right(t, grandparent(n));
    } else {
        assert (n == n->parent->right && n->parent == grandparent(n)->right);
        rotate_left(t, grandparent(n));
    }
}
zinline void insert_case4(zbtree_t *t, zbtnode_t* n) {
    if (n == n->parent->right && n->parent == grandparent(n)->left) {
        rotate_left(t, n->parent);
        n = n->left;
    } else if (n == n->parent->left && n->parent == grandparent(n)->right) {
        rotate_right(t, n->parent);
        n = n->right;
    }
    insert_case5(t, n);
}

zinline void insert_case3(zbtree_t *t, zbtnode_t* n) {
    if (node_color(uncle(n)) == RED) {
        n->parent->color = BLACK;
        uncle(n)->color = BLACK;
        grandparent(n)->color = RED;
        insert_case1(t, grandparent(n));
    } else {
        insert_case4(t, n);
    }
}

zinline void insert_case2(zbtree_t *t, zbtnode_t* n) {
    if (node_color(n->parent) == BLACK)
        return;
    else
        insert_case3(t, n);
}

static void insert_case1(zbtree_t *t, zbtnode_t* n) {
    if (n->parent == NULL)
        n->color = BLACK;
    else
        insert_case2(t, n);
}


zbtnode_t* maximum_node(zbtnode_t* n) {
    assert (n != NULL);
    while (n->right != NULL) {
        n = n->right;
    }
    return n;
}
static void delete_case1(zbtree_t *t, zbtnode_t* n);
zinline void delete_case6(zbtree_t *t, zbtnode_t* n) {
    sibling(n)->color = node_color(n->parent);
    n->parent->color = BLACK;
    if (n == n->parent->left) {
        assert (node_color(sibling(n)->right) == RED);
        sibling(n)->right->color = BLACK;
        rotate_left(t, n->parent);
    }
    else
    {
        assert (node_color(sibling(n)->left) == RED);
        sibling(n)->left->color = BLACK;
        rotate_right(t, n->parent);
    }
}

zinline void delete_case5(zbtree_t *t, zbtnode_t* n) {
    if (n == n->parent->left &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == RED &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        sibling(n)->left->color = BLACK;
        rotate_right(t, sibling(n));
    }
    else if (n == n->parent->right &&
             node_color(sibling(n)) == BLACK &&
             node_color(sibling(n)->right) == RED &&
             node_color(sibling(n)->left) == BLACK)
    {
        sibling(n)->color = RED;
        sibling(n)->right->color = BLACK;
        rotate_left(t, sibling(n));
    }
    delete_case6(t, n);
}

zinline void delete_case4(zbtree_t *t, zbtnode_t* n) {
    if (node_color(n->parent) == RED &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == BLACK &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        n->parent->color = BLACK;
    }
    else
        delete_case5(t, n);
}

zinline void delete_case3(zbtree_t *t, zbtnode_t* n) {
    if (node_color(n->parent) == BLACK &&
        node_color(sibling(n)) == BLACK &&
        node_color(sibling(n)->left) == BLACK &&
        node_color(sibling(n)->right) == BLACK)
    {
        sibling(n)->color = RED;
        delete_case1(t, n->parent);
    }
    else
        delete_case4(t, n);
}

zinline void delete_case2(zbtree_t *t, zbtnode_t* n) {
    if (node_color(sibling(n)) == RED) {
        n->parent->color = RED;
        sibling(n)->color = BLACK;
        if (n == n->parent->left)
            rotate_left(t, n->parent);
        else
            rotate_right(t, n->parent);
    }
    delete_case3(t, n);
}

static void delete_case1(zbtree_t *t, zbtnode_t* n) {
    if (n->parent == NULL)
        return;
    else
        delete_case2(t, n);
}

zerr_t zrbtree_insert(zbtree_t *t, zbtnode_t *inserted_node) {
    inserted_node->color = RED;
    zspin_lock(&t->spin);
    if (t->root == NULL) {
        t->root = inserted_node;
    } else {
        zbtnode_t* n = t->root;
        //ZDBG("before positon insert:");
        while (1) {
            int comp_result = zrbtree_compare_node(t, n, inserted_node);
            if (comp_result == 0) {
                if(t->multiple){
                    /* multi insert */
                    while(n->next){
                        n = n->next;
                    }
                    n->next = inserted_node; /* support multi insert if inserted_node is a list */
                    zspin_unlock(&t->spin);
                    //ZDBG("mulit item");
                    return 0;
                }else{
                    ZDBG("equal item refer<ptr:%p key:%d> <==> inserted_node<ptr:%p key:%d>",
                         n, zrbn_key(n), inserted_node, zrbn_key(inserted_node));
                    zspin_unlock(&t->spin);
                    return -1;
                }
            } else if (comp_result > 0) {
                if (n->left == NULL) {
                    n->left = inserted_node;
                    break;
                } else {
                    n = n->left;
                }
            } else {
                assert (comp_result < 0);
                if (n->right == NULL) {
                    n->right = inserted_node;
                    break;
                } else {
                    n = n->right;
                }
            }
        }
        inserted_node->parent = n;
    }
    //ZDBG("before rebalance insert: ptr<%p>", inserted_node);
    //zrbtree_print(t);
    insert_case1(t, inserted_node);
    //ZDBG("after rebalance insert: ptr<%p>", inserted_node);
    //zrbtree_print(t);
    //verify_properties(t);
    //ZDBG("insert node<ptr:%p>", inserted_node);
    zspin_unlock(&t->spin);
    return 0;
}

zerr_t zrbtree_erase(zbtree_t *t, zbtnode_t *key) {
    zbtnode_t* child;
    int comp_result;
    zbtnode_t *n;

    zspin_lock(&t->spin);
    /* look up node */
    //ZDBG("before erase<%d>:", zrbn_key(key));
    //zrbtree_print(t);
    n = t->root;
    while (n != NULL) {
        comp_result = zrbtree_compare_node(t, n, key);
        if (comp_result == 0) {
            //zspin_unlock(&t->spin);
            break;
        } else if (comp_result > 0) {
            n = n->left;
        } else {
            assert(comp_result < 0);
            n = n->right;
        }
    }
    if (n == NULL){
        zspin_unlock(&t->spin);
        return -1;
    }
    /* erase node */
    if (n->left != NULL && n->right != NULL) {
        zbtnode_t* pred = maximum_node(n->left);
#if 0
        memcpy(n->data, pred->data, t->data_size);
        if(t->multiple){
            child = n->next;
            n->next = pred->next;
            pred->next = child;
        }
        n = pred;
#else
        zbtnode_t *tmp = pred->parent;
        enum rbtree_node_color tmp_color = pred->color;
        pred->color = n->color;
        n->color = tmp_color;

        if(pred->parent){
            pred == pred->parent->left ? (pred->parent->left = n) : (pred->parent->right = n);
        }
        if(n->parent){
            n == n->parent->left ? (n->parent->left = pred) : (n->parent->right = pred);
        }
        pred->parent = n->parent;
        n->parent = tmp;

        tmp = pred->left;
        if(pred->left){
            pred->left->parent = n;
        }
        if(n->left){
            n->left->parent = pred;
        }
        pred->left = n->left;
        n->left = tmp;

        tmp = pred->right;
        if(pred->right){
            pred->right->parent = n;
        }
        if(n->right){
            n->right->parent = pred;
        }
        pred->right = n->right;
        n->right = tmp;
        memcpy(n->data, pred->data, t->data_size);
        if(!pred->parent){
            t->root = pred;
        }
        //ZDBG("after swap n<%d>:", zrbn_key(n));
        //zrbtree_print(t);
#endif
    }

    assert(n->left == NULL || n->right == NULL);
    //child = n->right == NULL ? n->left  : n->right;
    child = n->right ? n->right : n->left;
    if (node_color(n) == BLACK) {
        n->color = node_color(child);
        delete_case1(t, n);
    }
    replace_node(t, n, child);
    if (n->parent == NULL && child != NULL)
        child->color = BLACK;
    while(n){
        /* multiple recycle */
        //ZDBG("push_node<ptr:%p>", n);
        zrbtree_push_node(t, &n);
        n = n->next;
    }
    //ZDBG("after erase<%d>:", zrbn_key(key));
    //zrbtree_print(t);
    //verify_properties(t);
    zspin_unlock(&t->spin);
    return 0;
}

zerr_t zrbtree_erasex(zbtree_t *t, zbtnode_t *key, zoperate compare){
    zbtnode_t* child = NULL;
    zbtnode_t *prev = NULL;
    zbtnode_t* n = NULL;
    zerr_t ret = 0;
    zerr_t comp_result = ZEQUAL;

    zspin_lock(&t->spin);
    /* look up node */
    n = t->root;
    while (n != NULL) {
        comp_result = zrbtree_compare_node(t, n, key);
        if (comp_result == 0) {
            zspin_unlock(&t->spin);
            break;
        } else if (comp_result > 0) {
            n = n->left;
        } else {
            assert(comp_result < 0);
            n = n->right;
        }
    }
    if (n == NULL){
        zspin_unlock(&t->spin);
        return -1;
    }
    /* erase node */
    /*
     * compare equal node
     */
    child = n;
    prev = NULL;
    while(child){
        if(ZEQUAL == (ret = compare((zvalue_t)child->data, NULL, (zvalue_t)key->data))){
            if(prev){
                prev->next = child->next; /* just erase node from list */
                zrbtree_push_node(t, &child);
            }
            break;
        }
        prev = child;
        child = child->next;
    }
    if(ZEQUAL == ret){
        if(prev != NULL){
            zspin_unlock(&t->spin);
            return 0;
        }//else{erase single node. as fllow}
    }else{
        /* Not find node. do nothing */
        zspin_unlock(&t->spin);
        return -1;
    }
    /*
     * erase single node.
     */
    if (n->left != NULL && n->right != NULL) {
        zbtnode_t* pred = maximum_node(n->left);
        //n->key   = pred->key;
#if 0
        memcpy(n->data, pred->data, t->data_size);
        if(t->multiple){
            child = n->next;
            n->next = pred->next;
            pred->next = child;
        }
        n = pred;
#else
        zbtnode_t *tmp = pred->parent;
        enum rbtree_node_color tmp_color = pred->color;
        pred->color = n->color;
        n->color = tmp_color;

        if(pred->parent){
            pred == pred->parent->left ? (pred->parent->left = n) : (pred->parent->right = n);
        }
        if(n->parent){
            n == n->parent->left ? (n->parent->left = pred) : (n->parent->right = pred);
        }
        pred->parent = n->parent;
        n->parent = tmp;

        tmp = pred->left;
        if(pred->left){
            pred->left->parent = n;
        }
        if(n->left){
            n->left->parent = pred;
        }
        pred->left = n->left;
        n->left = tmp;

        tmp = pred->right;
        if(pred->right){
            pred->right->parent = n;
        }
        if(n->right){
            n->right->parent = pred;
        }
        pred->right = n->right;
        n->right = tmp;
        memcpy(n->data, pred->data, t->data_size);
        if(!pred->parent){
            t->root = pred;
        }
#endif
    }

    assert(n->left == NULL || n->right == NULL);
    child = n->right == NULL ? n->left  : n->right;
    if (node_color(n) == BLACK) {
        n->color = node_color(child);
        delete_case1(t, n);
    }
    replace_node(t, n, child);
    if (n->parent == NULL && child != NULL)
        child->color = BLACK;
    while(n){
        /* multiple recycle */
        zrbtree_push_node(t, &n);
        n = n->next;
    }
    //verify_properties(t);
    zspin_unlock(&t->spin);
    return 0;
}

zerr_t zrbtree_pop_front(zbtree_t *t, zbtnode_t **node,
                         zoperate condition, zvalue_t hint){
    zbtnode_t* child;
    zbtnode_t *n;

    zspin_lock(&t->spin);

    n = t->root;
    if (n == NULL){
        zspin_unlock(&t->spin);
        return -1;
    }
    while (n->left) {
        n = n->left; /* get minimum */
    }

    if(condition && (ZOK != condition((zvalue_t)n->data, NULL, hint))){
        zspin_unlock(&t->spin);
        return -1;
    }
    /* erase node */
    assert(n->left == NULL || n->right == NULL);
    //child = n->right == NULL ? n->left  : n->right;
    child = n->right ? n->right : n->left;
    if (node_color(n) == BLACK) {
        n->color = node_color(child);
        delete_case1(t, n);
    }
    replace_node(t, n, child);
    if (n->parent == NULL && child != NULL)
        child->color = BLACK;
    zspin_unlock(&t->spin);
    *node = n;
    (*node)->left = (*node)->right = (*node)->parent = NULL;
    return 0;
}