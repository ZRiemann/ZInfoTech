#include <unistd.h>
#include <sys/types.h>
#include <zit/container/queue.h>

void tmulti_define(){
    zcontainer_t que;
    zque1_create(&que, 0);

    zque1_destroy(que);
}