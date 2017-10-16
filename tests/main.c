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
#include "tlist.h"
#include "ttask.h"
#include "tmemory.h"

int main(int argc, char** argv){
#if 1
    ztrace_reg(ztrace_console, 0);
#else
    ztrace_logctl("ttask.log", 512000000);
    ztrace_reg(ztrace_log, 0);
#endif
    zdbg("\n queue <chunk_size> <value_size>"
         "\n tree <numbers>"
         "\n list <numbers>"
         "\n task [message | throughput] <numbers>"
         "\n memory"
         "\n epoll echo_svr_cli <ehco_number> <client_threads> <conn_per_thr>"
         "\n");
    if(argc >= 2 && strcmp("tree", argv[1]) == 0){
        ttree(argc, argv);
    }else if(argc >= 2 && strcmp("memory", argv[1]) == 0){
        tmemory(argc, argv);
    }else if(argc >= 2 && strcmp("list", argv[1]) == 0){
        tlist(argc, argv);
    }else if(argc >= 2 && strcmp("task", argv[1]) == 0){
        ttask(argc, argv);
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

