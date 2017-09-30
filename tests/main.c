#include <stdio.h>
#include <zit/base/error.h>
#include <zit/base/trace.h>
#include <zit/base/time.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/traceconsole.h>
#include <zit/thread/thread.h>

#include <string.h>
#include <stdlib.h>
#include <zit/base/time.h>


#include "tqueue.h"
#include "tepoll.h"
#include "ttree.h"

int main(int argc, char** argv){
    ztrace_reg(ztrace_console, 0);

    zdbg("\n queue <chunk_size> <value_size>"
         "\n tree <numbers>"
         "\n epoll echo_svr_cli <ehco_number> <client_threads> <conn_per_thr>");
    if(argc >= 2 && strcmp("tree", argv[1]) == 0){
        ttree(argc, argv);
    }else if(argc >= 2 && strcmp("epoll", argv[1]) == 0){
        tepoll(argc, argv);
    }else if(argc >= 2 && strcmp("queue", argv[1]) == 0){
        tqueue(argc, argv);
    }else{
        zdbg("param error");
    }

    zdbg("all testing down.");
    return 0;
}

