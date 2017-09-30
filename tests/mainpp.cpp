/**
 * @file tests/main.cpp
 * @brief contrast c++ lib with libzit;
 */
#include <stdlib.h>
#include <zit/thread/spin.h>
#include <zit/thread/thread.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/utility/traceconsole.h>
#include <string.h>
#include <queue>
#include <list>
#include "tqueue.h"
#include "ttree.h"

int main(int argc, char** argv){
    ztrace_reg(ztrace_console, 0);
    if(argc >= 2 && strcmp("tree", argv[1]) == 0){
        ttree(argc, argv);
    }else if(argc >= 2 && strcmp("queue", argv[1]) == 0){
        tqueue(argc, argv);
    }else{
        zdbg("param error");
    }
    zdbg("all testing down.");
    return 0;
}