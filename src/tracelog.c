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
static int zg_logprefixsize = 0;
static long zg_logsize = ZLOG_FILE_SIZE;
static long zg_loglen = 0;
static char zg_logname[ZLOG_NAME_SIZE] = "ZInfoTech.log";
static char zg_logname_backup[ZLOG_NAME_SIZE] = "ZInfoTech.log.log";

static int ztrace_logfixname(){
  // set log file path seem to module path
  if((NULL == strchr(zg_logname, '\\')) || (NULL == strchr(zg_logname, '/'))){
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

static int ztrace_logbackup(const char* msg)
{
    int ret = ZEOK;
    //ret = HTK_TraceLogOpen();
    //HTK_VERIFY_ERR(HTK_EOK, ret);
    zg_loglen += zg_logprefixsize;
    zg_loglen += strlen(msg);
    if( zg_loglen < zg_logsize )
    {
        return ret;
    }
    ret = fclose(zg_logpf);
    //HTK_VERIFY_EQUAL(EOF, ret, HTK_ECALL_FUN_FAIL);
    ret = remove(zg_logname_backup);
    //HTK_VERIFY_EQUAL(-1, ret, HTK_ECALL_FUN_FAIL);    // first time backup will return -1;
    ret = rename(zg_logname, zg_logname_backup);
    //HTK_VERIFY_ERR(0,ret);
    zg_logpf = fopen(zg_logname,"w");
    //HTK_VERIFY_EQUAL(NULL, g_pfLog, HTK_ECALL_FUN_FAIL);
    zg_loglen = 0;
    return ret;
}

int ztrace_logctl(const char* fname, int logsize){
  if(NULL != zg_logpf){
    fclose(zg_logpf);
  }
  if(fname && (strlen(fname) < ((size_t)ZLOG_NAME_SIZE-4)) )
  {
    sprintf(zg_logname,"%s",fname);
    sprintf(zg_logname_backup, "%s.log",fname);
  }
  if( (logsize > 1023) && (logsize < 512*1024*1024) )
  {
    zg_logsize = logsize;
  }
  return ZEOK;
}

int ztrace_log(int level, void* usr, const char* msg){
  int ret = ZEOK;
 
  const char* szLevel = " DBG:";
  switch(level){
    case ZTRACE_LEVEL_ERR:
        szLevel = " ERR:";
        break;
    case ZTRACE_LEVEL_WAR:
        szLevel = " WAR:";
        break;
    case ZTRACE_LEVEL_MSG:
        szLevel = " MSG:";
        break;
    case ZTRACE_LEVEL_DBG:
        szLevel = " DBG:";
        break;
    default:
        break;
    }
    // CAUTION: NEED A LOCK FOR MULTITHREAD...
    // [TASK DELAY] lock log file for write.

    ret = ztrace_logopen();
    if( ZEOK == ret )
    {
        static char szNow[64] = {0};
        zstr_systime_now(szNow,ZTP_MILLISEC);
        if( 0 == zg_logprefixsize)
        {
            zg_logprefixsize = strlen(szNow);
            zg_logprefixsize += strlen(szLevel);
            zg_logprefixsize ++;
        }
        fprintf(zg_logpf,"%s%s %s\n",szNow,szLevel,msg);
	//fflush(zg_logpf); // ztrace_logctl(NULL,0); will close and flush.
        ret = ztrace_logbackup(msg);
    }
    return ret;
}
