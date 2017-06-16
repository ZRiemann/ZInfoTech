#include "export.h"
#include <stdio.h>
#include <string.h>
#include <zit/base/error.h>
#include <zit/base/time.h>
#include <zit/base/trace.h>
#include <zit/utility/tracelog.h>
#include <zit/utility/module.h>

#define ZLOG_FILE_SIZE 1024*1024*16
#define ZLOG_NAME_SIZE 256

static FILE* zg_logpf = NULL;
//static int zg_logprefixsize = 0;
static long zg_logsize = ZLOG_FILE_SIZE;
static long zg_loglen = 0;
static char zg_logname[ZLOG_NAME_SIZE] = "ZInfoTech.log";
static char zg_logname_backup[ZLOG_NAME_SIZE] = "ZInfoTech.log.log";

static int ztrace_logfixname(){
  // set log file path seem to module path
  if((NULL == strchr(zg_logname, '\\')) && (NULL == strchr(zg_logname, '/'))){
    char path[ZLOG_NAME_SIZE];
    if(ZEOK == zmodule_name(path, NULL)){
      strcat(path,"/");
      strcat(path, zg_logname);
      strcpy(zg_logname, path);
      strcat(path, ".log");
      strcpy(zg_logname_backup, path);
    }
  }
  return ZEOK;
}

static int ztrace_logopen()
{
  int ret = ZEOK;
  if( NULL == zg_logpf ){
    ztrace_logfixname();
    zg_logpf = fopen(zg_logname,"a");
    if( NULL != zg_logpf ){
      fseek(zg_logpf,0,SEEK_END);
      zg_loglen = ftell(zg_logpf);
    }else{
      ret = ZEFUN_FAIL;
    }
  }
  return ret;
}

static int ztrace_logbackup(const char* msg, int len)
{
  int ret = ZEOK;
  zg_loglen += len;
  if( zg_loglen < zg_logsize ){
    return ret;
  }
  ret = fclose(zg_logpf);
  ret = remove(zg_logname_backup);
  ret = rename(zg_logname, zg_logname_backup);
  zg_logpf = fopen(zg_logname,"w");
  zg_loglen = 0;
  return ret;
}

int ztrace_logctl(const char* fname, int logsize){
  if(NULL != zg_logpf){
    fclose(zg_logpf);
    zg_logpf = NULL;
  }
  if(fname && (strlen(fname) < ((size_t)ZLOG_NAME_SIZE-4)) ){
    sprintf(zg_logname,"%s",fname);
    sprintf(zg_logname_backup, "%s.log",fname);
  }
  if( (logsize > 1023) && (logsize < 512*1024*1024) ){
    zg_logsize = logsize;
  }else{
    zg_logsize = 16*1024*1024;
  }
  return ZEOK;
}

int ztrace_log(int len, void* usr, const char* msg){
  int ret;
  ret = ztrace_logopen();
  if( ZEOK == ret ){
    fputs(msg, zg_logpf);
    //fflush(zg_logpf); // ztrace_logctl(NULL,0); will close and flush.
    ztrace_logbackup(msg, len);
  }
  return ZOK;
}

void ztrace_logflush(){
  if(zg_logpf){
    fflush(zg_logpf);
  }
}
