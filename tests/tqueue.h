#ifndef _ZTST_QUEUE_H_
#define _ZTST_QUEUE_H_

#include <zit/container/base/queue.h>
#ifdef __cplusplus
#include <queue>
#endif

static zerr_t tqueue_print_i32(ZOP_ARG){
    static int max= 0;
    printf("%d%c", *(int32_t*)in, ((++max) & 0x1f) ? ' ' : '\n');
    return ZEOK;
}
static void tqueue_i32(zqueue_t *que){
    ztick_t tick = NULL;
    int32_t i = 0;
    int32_t *pi = NULL;
    int32_t *pstep = NULL;
    int32_t max_value = 100000000;
    ZDBG("\n--------------------------------------------------------------------------------\n"
         "test base queue<int32_t>:");
    ZDBG("push 0~1023");
    for(i=0; i<1024; ++i){
        zqueue_push(que, (zvalue_t)&i);
    }
    zqueue_foreach(que, tqueue_print_i32, NULL);

    ZDBG("pop 0~255:");
    for(i=0; i<256; ++i){
        zqueue_pop(que, NULL);
    }
    zqueue_foreach(que, tqueue_print_i32, NULL);

    ZDBG("pop_back 255 items:");
    for(i=0; i<256; ++i){
        zqueue_pop_back(que, NULL);
    }
    zqueue_foreach(que, tqueue_print_i32, NULL);

    ZDBG("pop all:");
    while(ZEOK == zqueue_pop(que, NULL));

    ZDBG("push array %d value:", max_value);
    pstep = pi = (int32_t*)calloc(max_value, sizeof(int32_t));
    tick = ztick();
    for(i=0; i<max_value; ++i){
#if 1
        pstep[i] = i;
#else
        *pstep = i;
        ++pstep;
#endif
    }
    ztock(tick, NULL, NULL);
    free(pi);

    ZDBG("push %d int32 value:", max_value);
    tick = ztick();
    for(i=0; i<max_value; ++i){
        zqueue_push(que, (zvalue_t)&i);
    }
    ztock(tick, NULL, NULL);
    while(ZEOK == zqueue_pop(que, NULL));

#ifdef __cplusplus
    ZDBG("queue push %d int32 value:", max_value);
    std::queue<int32_t> stdq;
    tick = ztick();
    for(i=0; i<max_value; ++i){
        stdq.push(i);
    }
    ztock(tick, NULL, NULL);
#endif
}

static void tqueue_i64(zqueue_t *que){
    
}

static void tqueue_float(zqueue_t *que){
    
}

static void tqueue_double(zqueue_t *que){
    
}

static void tqueue(int argc, char **argv){
    zqueue_t *que = NULL;
    zerr_t ret = ZOK;
    int32_t chunk_size = atoi(argv[2]);
    int32_t value_size = atoi(argv[3]);
    if(argc != 4){
        ZERRC(ZPARAM_INVALID);
        return;
    }
    ZDBG("\n================================================================================\n");
    ret = zqueue_create(&que, chunk_size, value_size); ZDBGC(ret);
    if(value_size == (int32_t)sizeof(int32_t)){
        tqueue_i32(que);
    }else if(value_size == (int32_t)sizeof(int64_t)){
        tqueue_i64(que);
    }else if(value_size == (int32_t)sizeof(float)){
        tqueue_float(que);
    }else if(value_size == (int32_t)sizeof(double)){
        tqueue_double(que);
    }
    zqueue_destroy(que);ZDBGC(ZEOK);
}

#endif