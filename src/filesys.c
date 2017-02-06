/**@file filesys.c
 * @brief file io and directory operates
 */
#include "export.h"
#include <zit/base/filesys.h>
#include <zit/base/trace.h>
#include <string.h>

zfd_t zfopen(const char *fname, int flag, int mode){
  zfd_t fd;
#ifdef ZSYS_POSIX
  if(O_CREAT & flag){
    fd = open(fname, flag, (mode_t)mode);
  }else{
    fd = open(fname, flag);
  }
  
  if(ZINVALID_FD == fd){
    ZERRC(errno);
  }else{
    ZDBG("zfopen(fname<%s>, flag<%d>, mode<%d>", fname, flag, mode);
  }
#else
  int access;
  int create;
  if(O_CREAT & flag){
    if(O_TRUNC & flag){
      create = OPEN_ALWAYS;
    }else{
      create = CREATE_ALWAYS;
    }
  }else{
    if(O_TRUNC & flag){
      create = TRUNCATE_EXISTING;
    }else{
      create = OPEN_EXISTING;
    }
  }
  if(O_RDONLY & flag){
    access = GENERIC_READ;
  }
  if(O_WRONLY & flag){
    access = GENERIC_WRITE;
  }
  if(O_RDWR & flag){
    access = GENERIC_READ | GENERIC_WRITE;
  }

  fd = CreateFile(fname, access, FILE_SHARE_READ, NULL, create, FILE_ATTRIBUTE_NORMAL, NULL); 

  if(ZINVALID_FD == fd){
    ZERRC(GetLastError());
  }else{
    ZDBG("zfopen(fname<%s>, flag<%d>, mode<%d>", fname, flag, mode);
  }
#endif
  return fd;
}

int zfclose(zfd_t fd){
  int ret;
#ifdef ZSYS_POSIX
  if(-1 == (ret = close(fd))){
    ZERRC(errno);
  }else{
    ZERRC(ZOK);
  }
#else
  if(0 == (ret = CloseHandle(fd))){
    ZERRC(GetLastError());
  }else{
    ZERRC(ZOK);
  }
#endif
  return ret;
}


int zfread(zfd_t fd, void *buf, int nbytes){
  int ret;
#ifdef ZSYS_POSIX
  ret = read(fd, buf, nbytes);
  if(0 == ret){
    ZDBG("EOF: Read to end of file");
  }else if(-1 == ret){
    ZERRC(errno);
  }else{
    ZDBG("Read %d bytes", ret);
  }
#else
  int nread;
  ret = ReadFile(fd, buf, nbytes, &nread, NULL);
  if(0 == ret){
    ZERRC(GetLastError());
    ret = -1;
  }else if(0 == nread){
    ZDBG("EOF: Read to end of file");
    ret = nread;
  }else{
    ZDBG("Read %d bytes", nread);
    ret = nread;
  }
#endif
  return ret;
}

int zfreadx(zfd_t fd, void *buf, int nbytes){
  int ret;
  int remain;
  int nread;
  ZASSERT(!buf || nbytes<0);
  
  remain = nbytes;
#ifdef ZSYS_POSIX
  do{
    nread = read(fd, buf+(nbytes-remain), remain);
    if(0 == nread){
      ZDBG("EOF: Read to end of file");
      ret = ZEOF;
      break;
    }else if(-1 == nread){
      ZERRC(errno);
      ret = ZFUN_FAIL;
      break;
    }else{
      ZDBG("Read %d bytes", nread);
      remain -= nread;
      ret = ZOK;
    }
  }while(remain);
#else
  do{
    ret = ReadFile(fd, buf, nbytes, &nread, NULL);
    if(0 == ret){
      ZERRC(GetLastError());
      ret = ZFUN_FAIL;
    }else if(0 == nread){
      ZDBG("EOF: Read to end of file");
      ret = ZEOF;
    }else{
      ZDBG("Read %d bytes", nread);
      remain -= nread;
      ret = ZOK;
    }
  }while(remain);
#endif
  if(ret == ZOK){
    ret = nbytes - remain;
  }
  return ret;
}

int zfwrite(zfd_t fd, const void *buf, int nbytes){
  int ret;
  int remain;
  int nwrite;
  ZASSERT(!buf || nbytes < 0);

  remain = nbytes;
#ifdef ZSYS_POSIX
  do{
    nwrite = write(fd, buf+(nbytes-remain), remain);
    if(-1 == nwrite){
      ZERRC(errno);
      ret = ZFUN_FAIL;
      break;
    }else{
      ZDBG("Write %d bytes", nwrite);
      remain -= nwrite;
      ret = ZOK;
    }
  }while(remain);
#else
  do{
    ret = WriteFile(fd, buf+(nbytes-remain), remain, &nwrite, NULL);
    if(0 == ret){
      ZERRC(GetLastError());
      ret = ZFUN_FAIL;
    }else{
      ZDBG("Write %d bytes", nwrite);
      remain -= nwrite;
      ret = ZOK;
    }
  }while(remain);
#endif
  if(ZOK == ret){
    ret = nbytes-remain;
  }
  return ret;
}

int zmkdir(const char *dir, int mode){
  int ret;
  ZASSERT(!dir);
  ret = ZOK;
#ifdef ZSYS_POSIX
  ret = mkdir(dir, mode);
  if(-1 == ret){
    ZERRC(errno);
    ret = ZFUN_FAIL;
  }else{
    ret = chmod(dir, S_IRWXU|S_IRGRP|S_IROTH);
    if(-1 == ret){
      ZERRC(ret);
    }
    ret = ZOK;
  }
#else
  ret = ZNOT_SUPPORT;   
#endif
  ZERRC(ret);
  return ret;
}

int zchdir(const char *dir){
  int ret;
  ZASSERT(!dir);
  ret = ZOK;
#ifdef ZSYS_POSIX
  ret = chdir(dir);
  if(ret < 0){
    ZERRC(errno);
    ret = ZFUN_FAIL;
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  ZERRC(ret);
  return ret;
}

int zgetcwd(char *buf, size_t size){
#ifdef ZSYS_POSIX
  getcwd(buf, size);
#else
  return ZNOT_SUPPORT;
#endif
  return ZOK;
}

int zrmdir(const char *dir){
  int ret;
  ZASSERT(!dir);
#ifdef ZSYS_POSIX
  ret  = rmdir(dir);
  if(-1 == ret){
    ZERRC(errno);
    ret = ZFUN_FAIL;
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  ZERRC(ret);
  return ret;
}

int zrmfile(const char *fname){
  int ret;
  ZASSERT(!fname);
#ifdef ZSYS_POSIX
  ret  = unlink(fname);
  if(-1 == ret){
    ZERRC(errno);
    ret = ZFUN_FAIL;
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  ZERRC(ret);
  return ret;
}

int zfstat(const char *pathname, zfstat_t *stat){
  int ret;

  ZASSERT(!pathname || !stat);
#ifdef ZSYS_POSIX
  struct stat buf;
  ret = lstat(pathname, &buf);
  if(0 > ret){
    ZERRC(errno);
  }else{
    if(S_ISREG(buf.st_mode)){
      stat->mode = ZFMODE_REGULAR;
    }else if(S_ISDIR(buf.st_mode)){
      stat->mode = ZFMODE_DIR;
    }else if(S_ISCHR(buf.st_mode)){
      stat->mode = ZFMODE_CHAR;
    }else if(S_ISBLK(buf.st_mode)){
      stat->mode = ZFMODE_BLOCK;
    }else if(S_ISFIFO(buf.st_mode)){
      stat->mode = ZFMODE_FIFO;
    }else if(S_ISLNK(buf.st_mode)){
      stat->mode = ZFMODE_LNK;
    }else if(S_ISSOCK(buf.st_mode)){
      stat->mode = ZFMODE_SOCK;
    }else{
      stat->mode = ZFMODE_UNKNOWN;
    }
    
    stat->ino = buf.st_ino;
    stat->dev = buf.st_dev;
    stat->rdev = buf.st_rdev;
    stat->nlink = buf.st_nlink;
    stat->uid = buf.st_uid;
    stat->gid = buf.st_gid;
    stat->size = buf.st_size;
#ifndef ZUSE_TIMESPEC
    stat->atime = buf.st_atime;
    stat->mtime = buf.st_mtime;
    stat->ctime = buf.st_ctime;
#else
    stat->atime.tv_sec = buf.st_atime.tv_sec;
    stat->mtime.tv_sec = buf.st_mtime.tv_sec;
    stat->ctime.tv_sec = buf.st_ctime.tv_sec;
    stat->atime.tv_nsec = buf.st_atime.tv_nsec;
    stat->mtime.tv_nsec = buf.st_mtime.tv_nsec;
    stat->ctime.tv_nsec = buf.st_ctime.tv_nsec;
#endif    
    stat->blksize = buf.st_blksize;
    stat->blocks = buf.st_blocks;
    ret = ZOK;
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}

int zftw(char fullpath[512], cbzftw func, zvalue_t hint){
  int ret;
#ifdef ZSYS_POSIX
  //struct stat statbuf;
  zfstat_t statbuf;
  struct dirent *dirp;
  DIR *dp;
  int n;
  
  ret = 0;
  if(ZOK != zfstat(fullpath, &statbuf)){
    return func(fullpath, &statbuf, FTW_NS, hint);
  }
  if(ZFMODE_DIR != statbuf.mode){
    return func(fullpath, &statbuf, FTW_F, hint);
  }
  if(0 != (ret = func(fullpath, &statbuf, FTW_D, hint))){
    return ret;
  }
  n = strlen(fullpath);
  fullpath[n++] = '/';
  fullpath[n] = 0;
  
  if(NULL == (dp = opendir(fullpath))){
    return func(fullpath, &statbuf, FTW_DNR, hint);
  }
  
  while((dirp = readdir(dp)) != NULL){
    if(0==strcmp(dirp->d_name, ".") || 0==strcmp(dirp->d_name, ".."))continue;
    strcpy(&fullpath[n], dirp->d_name);
    if((ret = zftw(fullpath, func, hint)) != 0){
      break;
    }
  }
  fullpath[n-1] = 0;
  if(closedir(dp)<0){
    ZDBG("Can't close directory %s", fullpath);
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}

ZAPI int zftw_nr(char fullpath[512], cbzftw func, zvalue_t hint){
  int ret;
  int ftw_flag;
#ifdef ZSYS_POSIX
  //struct stat statbuf;
  zfstat_t statbuf;
  struct dirent *dirp;
  DIR *dp;
  int n;
  
  ret = 0;
  if(ZOK != zfstat(fullpath, &statbuf)){
    return func(fullpath, &statbuf, FTW_NS, hint);
  }
  if(ZFMODE_DIR != statbuf.mode){
    return func(fullpath, &statbuf, FTW_F, hint);
  }
  if(0 != (ret = func(fullpath, &statbuf, FTW_D, hint))){
    return ret;
  }
  n = strlen(fullpath);
  fullpath[n++] = '/';
  fullpath[n] = 0;
  
  if(NULL == (dp = opendir(fullpath))){
    return func(fullpath, &statbuf, FTW_DNR, hint);
  }
  
  while((dirp = readdir(dp)) != NULL){
    if(0==strcmp(dirp->d_name, ".") || 0==strcmp(dirp->d_name, ".."))continue;
    strcpy(&fullpath[n], dirp->d_name);
    if(ZOK != zfstat(fullpath, &statbuf)){
      ftw_flag = FTW_NS;
    }else if(ZFMODE_DIR != statbuf.mode){
      ftw_flag = FTW_F;
    }else{
      ftw_flag = FTW_D;
    }
    func(fullpath, &statbuf, ftw_flag, hint);
  }
  fullpath[n-1] = 0;
  if(closedir(dp)<0){
    ZDBG("Can't close directory %s", fullpath);
  }
#else
  ret = ZNOT_SUPPORT;
#endif
  return ret;
}
int print_zftw(const char *pathname, zfstat_t *stat, int ftw_flag){
  struct tm stm;
#ifndef ZUSE_TIMESPEC
  stm = *localtime(&stat->ctime);
  switch(ftw_flag){
  case FTW_F:
    zdbg("file[%d-%02d-%02d %02d:%02d:%02d]: %s",stm.tm_year+1900, stm.tm_mon, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, pathname);
    break;
  case FTW_D:
    zdbg("dir[%d-%02d-%02d %02d:%02d:%02d]: %s",stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, pathname);
    break;
  case FTW_DNR:
    zdbg("dirNR: %s", pathname);
    break;
  case FTW_NS:
    zdbg("zfstat() error for %s", pathname);
    break;
  default:
    ZERR("unknown type %d for pathname %s", ftw_flag, pathname);
  }
#else
  stm = *localtime(&stat->ctime.tv_sec);
  switch(ftw_flag){
  case FTW_F:
    zdbg("file[%d-%02d-%02d %02d:%02d:%02d]: %s",stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, pathname);
    break;
  case FTW_D:
    zdbg("dir[%d-%02d-%02d %02d:%02d:%02d]: %s",stm.tm_year+1900, stm.tm_mon+1, stm.tm_mday, stm.tm_hour, stm.tm_min, stm.tm_sec, pathname);
    break;
  case FTW_DNR:
    zdbg("dirNR: %s", pathname);
    break;
  case FTW_NS:
    zdbg("zfstat() error for %s", pathname);
    break;
  default:
    ZERR("unknown type %d for pathname %s", ftw_flag, pathname);
  }
#endif
  return 0;
}

