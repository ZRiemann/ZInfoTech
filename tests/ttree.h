#ifndef _ZTST_TREE_H_
#define _ZTST_TREE_H_

#include <zit/container/rbtree.h>
#include <zit/base/time.h>

static zerr_t ttree_print_key32(ZOP_ARG){
    if(in){
        printf("%d->", *(int32_t*)(((zbtnode_t*)in)->data));
    }
    return ZOK;
}
#ifdef __cplusplus
/*
 * std::map VS zit.zbtree
 */
#include <map>
static void ttree_vs_map(int cnt){
    zerr_t ret = ZEOK;
    zbtree_t *tree = 0;
    zbtnode_t *node = NULL;
    zbtnode_t *tmp_nod = NULL;
    int i = 0;
    int *insert_array = NULL;
    int insert_cnt = 0;
    int max_rand = cnt * 10;
    ztick_t tick = NULL;
    int key;

    /* prepare random insert nodes */
    ZDBG("Preparing %d rand keys: node_size<%d> value_size<%d>", cnt, sizeof(zbtnode_t), sizeof(int32_t));
    i = sizeof(int32_t);
    ret = zrbtree_create(&tree, i, 4096, NULL, 0); ZERRC(ret);/* 80KB */
    insert_array = (int*)calloc(cnt, sizeof(int));
    while(insert_cnt < cnt){
        if(ZOK != (zrbtree_get_node(tree, &node))){
            ZERRC(ZEMEM_INSUFFICIENT);
            break;
        }
        key = zrandin(max_rand);
        zrbn_key(node)  = key;
        if(ZOK == zrbtree_insert(tree, node)){
            insert_array[insert_cnt++] = key;
        }else{
            zrbtree_recycle_node(tree, &node);
            //ZDBG("push cnt<%d>", insert_cnt);
        }
    }
    zrbtree_destroy(tree);
    /* Insert nodes*/
    i = sizeof(zbtnode_t) + sizeof(int32_t);
    zrbtree_create(&tree, i, 4096, NULL, 0); ZERRC(ret);
    ZDBG("Begin insert %d random nodes to red-black tree:", cnt);
    tick = ztick();
    for(i=0; i<cnt; ++i){
        zrbtree_get_node(tree, &node);
        zrbn_key(node) = insert_array[i];
        zrbtree_insert(tree, node);
    }
    ztock(tick, NULL, NULL);
    if(cnt < 10000){
        zrbtree_print(tree);
    }
    ret= zrbtree_verify(tree);
    ZERRC(ret);

    ZDBG("Begin insert %d random nodes to map:", cnt);
    std::map<int, int> i2i;
    tick = ztick();
    for(i=0; i<cnt; ++i){
        i2i.insert(std::pair<int, int>(i, i));
    }
    ztock(tick, NULL, NULL);

    ZDBG("Begin find %d random nodes from red-black tree:", cnt);
    zrbtree_get_node(tree, &node);
    zrbtree_get_node(tree, &tmp_nod);
    tick = ztick();
    for(i=0; i<cnt; ++i){
        zrbn_key(tmp_nod) = insert_array[i];
        zrbtree_find(tree, tmp_nod, &node);
        //if(ZOK != zrbtree_find(tree, tmp_nod, &node)){
        //    ZDBG("not find data<%d>", insert_array[i]);
        //}
    }
    ztock(tick, NULL, NULL);
    zrbtree_recycle_node(tree, &node);
    zrbtree_recycle_node(tree, &tmp_nod);

    ZDBG("Begin find %d random nodes from map:", cnt);
    std::map<int, int>::iterator it;
    tick = ztick();
    for(i=0; i<cnt; ++i){
        it = i2i.find(insert_array[i]);
    }
    ztock(tick, NULL, NULL);
#if 0
    FILE *pf = fopen("tree.txt", "w");
    char buf[128];
    for(i=0; i<cnt; ++i){
        sprintf(buf, "%d\n", insert_array[i]);
        fputs(buf, pf);
    }
    fclose(pf);
#endif
    ZDBG("Begin erase %d random nodes from red-black tree:", cnt);
    zrbtree_get_node(tree, &tmp_nod);
    tick = ztick();
    for(i=0; i<cnt; ++i){
        //ZDBG("Erase key<%d>", insert_array[i]);
        zrbn_key(tmp_nod) = insert_array[i];
        zrbtree_erase(tree, tmp_nod);
        //zrbtree_print(tree);
    }
    ztock(tick, NULL, NULL);
    zrbtree_recycle_node(tree, &tmp_nod);

    ZDBG("Begin erase %d random nodes from map:", cnt);
    tick = ztick();
    for(i=0; i<cnt; ++i){
        i2i.erase(insert_array[i]);
    }
    ztock(tick, NULL, NULL);

    zrbtree_destroy(tree);
    if(cnt < 10000){
    ZDBG("test erase 1/2 and insert full loop");
    i = sizeof(zbtnode_t) + sizeof(int32_t);
    ret = zrbtree_create(&tree, i, 4096, NULL, 0);

    for(i=0; i<cnt; ++i){
        zrbtree_get_node(tree, &node);
        zrbn_key(node) = insert_array[i];
        zrbtree_insert(tree, node);
    }
    zrbtree_print(tree);
    printf("\n");
    zrbtree_foreach(tree->root, 0, ttree_print_key32, NULL);
    printf("\n");
    zrbtree_get_node(tree, &tmp_nod);
    for(int round = 0; round < 10000; ++round){
        if(!(round & 0xff)){
            printf("round<%d>", round);
        }
        for(i=cnt/2; i<cnt; ++i){
            //zrbtree_print(tree);
            zrbn_key(tmp_nod) = insert_array[i];
            if(ZOK != zrbtree_erase(tree, tmp_nod)){
                ZDBG("FAILED Erase array[%d]:<%d>", i, insert_array[i]);
            }
        }
        for(i=cnt/2; i<cnt; ++i){
            if(ZOK != (zrbtree_get_node(tree, &node))){
                ZERRC(ZEMEM_INSUFFICIENT);
                break;
            }
            while(1){
                key = zrandin(max_rand);
                zrbn_key(node)  = key;
                if(ZOK == zrbtree_insert(tree, node)){
                    insert_array[i] = key;
                    break;
                }
            }
        }
        //zrbtree_print(tree);
    }
    zrbtree_recycle_node(tree, &tmp_nod);
    zrbtree_print(tree);
    printf("\n");
    zrbtree_foreach(tree->root, 0, ttree_print_key32, NULL);
    printf("\n");
    ret = zrbtree_verify(tree);
    ZERRCX(ret);
    zrbtree_destroy(tree);
    }

}
#endif /* __cplusplus*/

static void talloc(){
    zalloc_t *alc = 0;
    int32_t *i32 = 0;
    zalloc_create(&alc, (int)sizeof(int32_t), 1024, 0);
    zalloc_pop(alc, (zvalue_t*)&i32);
    *i32 = 32;
    zalloc_push(alc, (zvalue_t*)&i32);
    zalloc_pop(alc, (zvalue_t*)&i32);
    *i32 = 64;
    zalloc_destroy(alc);
}

static void tprint_tree(int key){
    zbtree_t *tree = NULL;
    zbtnode_t *node = NULL;
    zbtnode_t *tmp_nod = NULL;
    zbtnode_t *refer = NULL;

    zrbtree_create(&tree, sizeof(int32_t), 4096, NULL, 0);

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 30;
    zrbn_set_black(node);
    tree->root = node;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 70;
    zrbn_set_black(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 85;
    zrbn_set_black(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 90;
    zrbn_set_red(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 85 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 80;
    zrbn_set_red(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 85 */
    refer = refer->parent; /* 70 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 60;
    zrbn_set_red(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 65;
    zrbn_set_red(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 60 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 50;
    zrbn_set_black(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 55;
    zrbn_set_red(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 50 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 40;
    zrbn_set_red(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 50 */
    refer = refer->parent; /* 60 */
    refer = refer->parent; /* 70 */
    refer = refer->parent; /* 30 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 15;
    zrbn_set_black(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 20;
    zrbn_set_black(node);
    refer->right = node;
    node->parent = refer;
    refer = node;

    refer = refer->parent; /* 15 */
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 10;
    zrbn_set_black(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    zrbtree_get_node(tree, &node);
    zrbn_key(node) = 5;
    zrbn_set_red(node);
    refer->left = node;
    node->parent = refer;
    refer = node;

    zrbtree_print(tree);
    ZDBG("After erase: %d", key);
    zrbtree_get_node(tree, &tmp_nod);
    zrbn_key(tmp_nod) = key;
    zrbtree_erase(tree, tmp_nod);
    zrbtree_recycle_node(tree, &tmp_nod);
    zrbtree_print(tree);
    zrbtree_destroy(tree);
}

static zbtnode_t *zrbtree_key_node(zbtree_t *tree, int32_t key){
    zbtnode_t *node = NULL;
    zrbtree_get_node(tree, &node);
    zrbn_key(node) = key;
    return node;
}

static void ttree_insert(){
    zbtree_t *tree = NULL;
    zbtnode_t *node = NULL;

    zrbtree_create(&tree, sizeof(int32_t), 4096, NULL, 0);

    node = zrbtree_key_node(tree, 50);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 80);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 90);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 40);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 30);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 55);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 33);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 85);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 77);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 54);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 64);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 44);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 34);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 24);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    node = zrbtree_key_node(tree, 74);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 84);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 11);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 21);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 31);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 41);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 51);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 61);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 71);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 81);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 91);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 72);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);

    node = zrbtree_key_node(tree, 73);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 74);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 75);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 76);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 77);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 78);
    zrbtree_insert(tree, node);
    //zrbtree_print(tree);
    node = zrbtree_key_node(tree, 79);
    zrbtree_insert(tree, node);
    zrbtree_print(tree);

    for(int i=0; i<20; ++i){
        node = zrbtree_key_node(tree, i);
        zrbtree_insert(tree, node);
    }
    zrbtree_print(tree);
    printf("\n");
    zrbtree_foreach(tree->root, 0, ttree_print_key32, NULL);
    printf("\n");
    for(int i=0; i<100; ++i){
        if(ZOK == zrbtree_erase(tree, (zbtnode_t*)&i)){
            zrbtree_print(tree);
        }else{
            ZDBG("Erase %d faild", i);
        }
    }
    zrbtree_destroy(tree);
}
static void ttree(int argc, char **argv){
#ifdef __cplusplus
    int cnt = 1000000;
    if(argc >= 3){
        cnt = atoi(argv[2]);
    }
#endif
    ttree_insert();
    talloc();
    tprint_tree(30);
#ifdef __cplusplus
    ttree_vs_map(cnt);
#endif
}
#endif