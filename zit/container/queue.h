#ifndef _ZCONT_QUE_1R1W_H_
#define _ZCONT_QUE_1R1W_H_
/**
 * @file zit/container/queue.h
 * @brief An efficient queue implementation. ZeroMQ yqueue like.
 * @details
 *  - (NOT) thread safe for multithread push()/pop()
 *  - minimize number of allocations/deallocations needded.
 *  - support 1 thread push() and 1 thread pop() with no lock,
 *    NOT SUPPORT 1 thread push() and 1 thread pop_front() with no lock
 *    NOT SUPPORT 1 thread push_front() and 1 thread pop() with no lock
 *  - for_each() only access by 1 thread;
 *  - inline implement
 *  - It's about 20000000 by 1 push thread and 1 pop thread per second, on (vm)ubuntu17
 *    Try ($ make/release/zit_test queue_1r1w) interval:0.499194 10 million
 *    Contrast zpin+std::queue with queue_1r1w push/pop 10 million;
 *    Try ($ make/release/zpp_test queue_1r1w) interval:1.036969 10 million
 *  - use inline module to improve 50% performance.
 */
#include <zit/base/type.h>

ZC_BEGIN

ZAPI zerr_t zque1_create(zcontainer_t *cont, void *hint);
ZAPI zerr_t zque1_destroy(zcontainer_t cont);
ZAPI zerr_t zque1_push(zcontainer_t cont, zvalue_t in); // push back
ZAPI zerr_t zque1_pop(zcontainer_t cont, zvalue_t *out); // pop front
ZAPI zerr_t zque1_pushfront(zcontainer_t cont, zvalue_t in);
ZAPI zerr_t zque1_popback(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zque1_insert(zcontainer_t cont, zvalue_t in, zoperate compare, int condition);
ZAPI zerr_t zque1_erase(zcontainer_t cont, zvalue_t hint, zoperate compare, int condition);
ZAPI zerr_t zque1_foreach(zcontainer_t cont, zoperate op, zvalue_t hint);
ZAPI size_t zque1_size(zcontainer_t cont);
ZAPI zerr_t zque1_back(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zque1_front(zcontainer_t cont, zvalue_t *out);
ZAPI zerr_t zque1_swap(zcontainer_t *cont1, zcontainer_t *cont2);

ZC_END

#endif
