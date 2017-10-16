#ifndef _ZTST_LIST_H_
#define _ZTST_LIST_H_

#include <zit/container/list.h>

zerr_t dump_list_int(ZOP_ARG){
    printf("<%d>", *(int*)(((znod_t*)in)->data));
    return ZEOK;
}
void tlist(int argc, char **argv){
    zlist_t *list = NULL;
    znod_t *nod = NULL;
    zerr_t ret = ZOK;
    int i;

    ZDBG("Begin test list.");

    ret = zlist_create(&list, 512, sizeof(int));
    ZDBG("create list %s", zstrerr(ret));

    ZDBG("push 1024 integer value");
    for(i=0; i<1024; ++i){
        if(ZEOK != (ret = zlist_alloc_node(list, &nod))){
            ZERRC(ret);
            break;
        }
        *(int*)nod->data = i;
        zlist_push(list, nod);
    }

    ZDBG("print list:");
    printf("\n");
    zlist_foreach(list, dump_list_int, NULL);
    printf("\n");

    ZDBG("pop 500:");
    for(i=0; i<500; ++i){
        ret = zlist_pop(list, &nod);
        printf("<%d>", *(int*)nod->data);
        zlist_recycle_node(list, nod);
    }
    printf("\n");

    ZDBG("pop back 500:");
        for(i=0; i<500; ++i){
        ret = zlist_pop_back(list, &nod);
        printf("<%d>", *(int*)nod->data);
        zlist_recycle_node(list, nod);
    }
    printf("\n");

    ZDBG("print list:");
    printf("\n");
    zlist_foreach(list, dump_list_int, NULL);
    printf("\n");

    ZDBG("pop 500:");
    for(i=0; i<500; ++i){
        ret = zlist_pop(list, &nod);
        if(ZOK == ret){
            printf("<%d>", *(int*)nod->data);
            zlist_recycle_node(list, nod);
        }
    }
    printf("\n");

    zlist_destroy(list);
    ZDBG("destroy list OK");

    ZDBG("End test list");
}
#endif
