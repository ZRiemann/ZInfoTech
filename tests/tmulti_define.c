#include <unistd.h>
#include <sys/types.h>
#include <zit/container/queue.h>
#include <zit/container/array.h>

void tmulti_define(){
    zcontainer_t que = 0;
    zarray_t *array = 0;
    zque1_create(&que, 0);
    zarray_create(&array, 1024);
    zarray_destroy(array);
    zque1_destroy(que);
}