/*
 *
 * Copyright (c) 2016 by Z.Riemann ALL RIGHTS RESERVED
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Z.Riemann makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */
/**
 * @file src/daemoinize.c
 * @brief 
 * @note
 *  1. umask(0)
 *  2. fork(), parent exit();
 *  3. setsid();
 *  4. change wwork dir to /
 *  5. close fd not used
 *  6. open /dev/null
 */
#include "export.h"
#include <zit/base/trace.h>
#include <zit/utility/daemonize.h>
#ifdef ZSYS_POSIX
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#define exit _exit
#else
#include <Windows.h>
#endif

#ifdef ZSYS_POSIX

void daemoinize(const char *cmd){
  int i, fd0, fd1, fd2;
  pid_t pid;
  struct rlimit rl;
  struct sigaction sa;

  // Clear file creation mask.
  umask(0);

  // Get maximum number of file descriptions
  if(getrlimit(RLIMIT_NOFILE, &rl) < 0){
    exit(-1);
  }

  // Become a session leader to lose controlling TTY
  if((pid = fork()) < 0){
    exit(-2);
  }else if(pid != 0){
    exit(0); // exit parent
  }
  // children setsid.
  setsid();

  // Ensure future opens won't allocate controlling TTYs.
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if(sigaction(SIGHUP, &sa, NULL) < 0){
    exit(-3);
  }

  if((pid = fork()) < 0){
    exit(-2);
  }else if(pid != 0){
    exit(0); // exit parent
  }

  if(chdir("/root") < 0){
    exit(-4);
  }

  // Close all open file descriptors
  if(rl.rlim_max == RLIM_INFINITY){
    rl.rlim_max = 1024;
  }
  for(i = 0; i< rl.rlim_max; ++i){
    close(i);
  }

  // Attach file descriptors 0, 1 and 2 to /dev/null.
  fd0 = open("dev/null", O_RDWR);
  fd1 = dup(0);
  fd2 = dup(0);

  // Initialize the log file.
  openlog(cmd, LOG_CONS, LOG_DAEMON);
  if(fd0 != 0 || fd1 != 1 || fd2 != 2){
    syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
    exit(-1);
  }
  
}

#else
void daemoinize(const char *cmd){
}
#endif
