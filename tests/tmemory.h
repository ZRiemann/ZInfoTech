#ifndef _TST_MEMORY_H_
#define _TST_MEMORY_H_

#include <zit/memory/alloc.h>
#include <zit/memory/mem_pool.h>
#include <zit/memory/obj_pool.h>

static void tmemory_alloc(int argc, char **argv){
    zerr_t ret = ZOK;
    zalloc_t *alc = NULL;
    int block_size = 128;
    int capacity = 512;
    int limit_mem = 32*1024*1024;
    zptr_t ptr1 = NULL, ptr2 = NULL;
    zptr_t ptr11 = NULL, ptr22 = NULL;

    ZDBG("Begin testing zalloc_t...");

    ret = zalloc_create(&alc, block_size, capacity, limit_mem);
    zdbg("zalloc_create(alc:%p, block_size:%d, "
         "capacity:%d, limit_mem:%d) %s",
         alc, block_size, capacity, limit_mem, zstrerr(ret));

    ret = zalloc_pop(alc, &ptr1);
    zdbg("%s = zalloc_pop(alc:%p, *(&ptr1):%p);", zstrerr(ret), alc, ptr1);
    ret = zalloc_pop(alc, &ptr2);
    zdbg("%s = zalloc_pop(alc:%p, *(&ptr2):%p);", zstrerr(ret), alc, ptr2);
    if(block_size != (char*)ptr1 - (char*)ptr2){
        ZERR("Test Fail block_size:%d != (ptr1 - ptr2):%d", block_size, (char*)ptr2-(char*)ptr1);
    }else{
        ZMSG("Test zalloc_pop(..) OK");
    }

    zalloc_push(alc, &ptr1);
    zalloc_push(alc, &ptr2);
    zalloc_pop(alc, &ptr11);
    zalloc_pop(alc, &ptr22);
    if(ptr2 != ptr22 || ptr1 != ptr11){
        ZERR("Test push and pop fail.");
    }else{
        ZMSG("Test push and pop memory OK");
    }

    ret = zalloc_destroy(alc);
    zdbg("zalloc_destroy(alc:%p): %s", alc, zstrerr(ret));
    ZDBG("Eend testing zalloc_t.");
}

static void tmemory_mem_pool(int argc, char **argv){
    zerr_t ret = ZOK;
    zmem_pool_t *alc = NULL;
    zptr_t ptr1 = NULL, ptr2 = NULL;
    zptr_t ptr11 = NULL, ptr22 = NULL;

    ZDBG("Begin testing zalloc_t...");

    ret = zmem_pool_create(&alc);
    zdbg("zmem_pool_create(alc:%p) %s",
         alc, zstrerr(ret));

    ret = zmem_pool_pop(alc, &ptr1, 20);
    zdbg("%s = zmem_pool_pop(alc:%p, *(&ptr1):%p, size:20);",
         zstrerr(ret), alc, ptr1);
    ret = zmem_pool_pop(alc, &ptr2, 120);
    zdbg("%s = zmem_pool_pop(alc:%p, *(&ptr2):%p, size:120);",
         zstrerr(ret), alc, ptr2);

    zmem_pool_push(alc, &ptr1, 20);
    zmem_pool_push(alc, &ptr2, 120);
    zmem_pool_pop(alc, &ptr11, 20);
    zmem_pool_pop(alc, &ptr22, 120);
    if(ptr2 != ptr22 || ptr1 != ptr11){
        ZERR("Test push and pop fail.");
    }else{
        ZMSG("Test push and pop memory OK");
    }

    ret = zmem_pool_destroy(alc);
    zdbg("zmem_pool_destroy(alc:%p): %s", alc, zstrerr(ret));
    ZDBG("Eend testing zmem_pool_t.");
}

static void tmemory_obj_pool_fixed_ref(zobj_pool_t *pool){
    zerr_t ret = ZOK;
    zobj_t *obj = NULL;
    zobj_t *obj_clone = NULL;
    zobj_t *obj_reuse = NULL;

    ZDBG("Begin obj_pool_fixed_ref...");
    ret = zobj_pool_pop(pool, &obj, ZCLONE_MODE_REF);
    ZERRCX(ret);

    ret = obj->clone(obj, (zvalue_t*)&obj_clone, NULL);
    ZERRCX(ret);
    if(obj != obj_clone || obj->ref !=2){
        ZERR("Reference clone failed.");
    }else{
        zmsg("Reference clone OK.");
    }

    ret = obj->release(obj, NULL, NULL);
    if(obj->ref != 1){
        ZERR("Release: obj reference count FAIL");
    }else{
        zmsg("Release: obj reference count OK");
    }

    ret = obj_clone->release(obj_clone, NULL, NULL);
    ZERRCX(ret);
    if(obj_clone->ref != 0){
        ZERR("Release: obj_clone reference count FAIL");
    }else{
        zmsg("Release: obj_clone reference count OK");
    }

    ret = zobj_pool_pop(pool, &obj_reuse, ZCLONE_MODE_REF);
    ZERRCX(ret);
    if(obj_reuse != obj){
        ZERR("Reuse object FAIL");
    }else{
        zmsg("Reuse object OK");
    }
    obj->release(obj, NULL, NULL);
    ZDBG("End obj_pool_fixed_ref.");
}

static void tmemory_obj_pool_fixed_mem(zobj_pool_t *pool){
    zerr_t ret = ZOK;
    zobj_t *obj = NULL;
    zobj_t *obj_clone = NULL;
    zobj_t *obj_reuse = NULL;

    ZDBG("Begin obj_pool_fixed_mem...");
    ret = zobj_pool_pop(pool, &obj, ZCLONE_MODE_MEMCPY);
    ZERRCX(ret);

    ret = obj->clone(obj, (zvalue_t*)&obj_clone, NULL);
    ZERRCX(ret);
    if(obj == obj_clone){
        ZERR("Memory clone failed.");
    }else{
        zmsg("Memory clone OK.");
    }

    ret = obj->release(obj, NULL, NULL);
    if(obj->ref != 0){
        ZERR("Release: obj FAIL");
    }else{
        zmsg("Release: obj OK");
    }

    ret = obj_clone->release(obj_clone, NULL, NULL);
    ZERRCX(ret);
    if(obj_clone->ref != 0){
        ZERR("Release: obj_clone FAIL");
    }else{
        zmsg("Release: obj_clone OK");
    }

    ret = zobj_pool_pop(pool, &obj_reuse, ZCLONE_MODE_MEMCPY);
    ZERRCX(ret);
    if(obj_reuse != obj){
        ZERR("Reuse object FAIL");
    }else{
        zmsg("Reuse object OK");
    }
    obj->release(obj, NULL, NULL);

    ZDBG("End obj_pool_fixed_mem.");
}

static void tmemory_obj_pool_unfixed_ref(zobj_pool_t *pool){
    zerr_t ret = ZOK;
    zobj_t *obj = NULL;
    zobj_t *obj_clone = NULL;
    zobj_t *obj_reuse = NULL;

    ZDBG("Begin obj_pool_unfixed_ref...");
    ret = zobj_pool_pop_unfixed(pool, &obj, ZCLONE_MODE_REF, 120);
    ZERRCX(ret);

    ret = obj->clone(obj, (zvalue_t*)&obj_clone, NULL);
    ZERRCX(ret);
    if(obj != obj_clone || obj->ref !=2){
        ZERR("Reference clone failed.");
    }else{
        zmsg("Reference clone OK.");
    }

    ret = obj->release(obj, NULL, NULL);
    if(obj->ref != 1){
        ZERR("Release: obj reference count FAIL");
    }else{
        zmsg("Release: obj reference count OK");
    }

    ret = obj_clone->release(obj_clone, NULL, NULL);
    ZERRCX(ret);
    if(obj_clone->ref != 0){
        ZERR("Release: obj_clone reference count FAIL");
    }else{
        zmsg("Release: obj_cline reference count OK");
    }

    ret = zobj_pool_pop_unfixed(pool, &obj_reuse, ZCLONE_MODE_REF, 123);
    ZERRCX(ret);
    if(obj_reuse != obj){
        ZERR("Reuse object FAIL");
    }else{
        zmsg("Reuse object OK");
    }
    obj_reuse->release(obj_reuse, NULL, NULL);
    ZDBG("End obj_pool_unfixed_ref.");
}

static void tmemory_obj_pool_unfixed_mem(zobj_pool_t *pool){
    zerr_t ret = ZOK;
    zobj_t *obj = NULL;
    zobj_t *obj_clone = NULL;
    zobj_t *obj_reuse = NULL;

    ZDBG("Begin obj_pool_unfixed_mem...");
    ret = zobj_pool_pop_unfixed(pool, &obj, ZCLONE_MODE_MEMCPY, 120);
    ZERRCX(ret);

    ret = obj->clone(obj, (zvalue_t*)&obj_clone, NULL);
    ZERRCX(ret);
    if(obj == obj_clone){
        ZERR("Memory clone failed.");
    }else{
        zmsg("Memory clone OK.");
    }

    ret = obj->release(obj, NULL, NULL);
    if(obj->ref != 0){
        ZERR("Release: obj FAIL");
    }else{
        zmsg("Release: obj OK");
    }

    ret = obj_clone->release(obj_clone, NULL, NULL);
    ZERRCX(ret);
    if(obj_clone->ref != 0){
        ZERR("Release: obj_clone FAIL");
    }else{
        zmsg("Release: obj_clone OK");
    }

    ret = zobj_pool_pop_unfixed(pool, &obj_reuse, ZCLONE_MODE_MEMCPY, 123);
    ZERRCX(ret);
    if(obj_reuse != obj){
        ZERR("Reuse object FAIL");
    }else{
        zmsg("Reuse object OK");
    }
    obj->release(obj, NULL, NULL);

    ZDBG("End obj_pool_unfixed_mem.");
}
static void tmemory_obj_pool(int argc, char **argv){
    zerr_t ret = ZOK;
    zobj_pool_t *pool = NULL;

    ZDBG("Begin testing zobj_pool_t...");
    zmem_pool_init();
    ret = zobj_pool_create(&pool, 1024, 512, 512*1024*1024);
    ZERRCX(ret);

    tmemory_obj_pool_fixed_ref(pool);
    tmemory_obj_pool_fixed_mem(pool);
    tmemory_obj_pool_unfixed_ref(pool);
    tmemory_obj_pool_unfixed_mem(pool);

    ret = zobj_pool_destroy(pool);
    ZERRCX(ret);
    zmem_pool_fini();
    ZDBG("Eend testing zobj_pool_t.");
}

static void tmemory(int argc, char **argv){
    ZDBG("Begin testing memory pool...");

    tmemory_alloc(argc, argv);
    tmemory_mem_pool(argc, argv);
    tmemory_obj_pool(argc, argv);

    ZDBG("End testing memory pool.");
}

#endif
