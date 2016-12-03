/**@file filesys.c
 * @brief file io and directory operates
 */
#include "export.h"
#include <zit/base/filesys.h>
#include <zit/base/trace.h>

zfd_t zfopen(const char *fname, int flag, int mode){
  zfd_t fd;
#ifdef ZSYS_POSIX
  if(O_CREAT & flag){
    fd = open(fname, flag, (mode_t)mode);
  }else{
    fd = open(fname, flag);
  }
  
  if(INVALID_FD == fd){
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

  if(INVALID_FD == fd){
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

int zfwrite(zfd_t fd, const void *buf, int nbytes){
  int ret;
#ifdef ZSYS_POSIX
  ret = write(fd, buf, nbytes);
  if(-1 == ret){
    ZERRC(errno);
  }else{
    ZDBG("Write %d bytes", ret);
  }
#else
  int nwrite;
  ret = WriteFile(fd, buf, nbytes, &nwrite, NULL);
  if(0 == ret){
    ZERRC(GetLastError());
    ret = -1;
  }else{
    ZDBG("Write %d bytes", nwrite);
    ret = nwrite;
  }
#endif
  return ret;
}

ZAPI int zfstat(const char *pathname, zfstat_t *stat){
  int ret;
  ret = ZOK;
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
    // atime
    // mtime;
    // ctime;
    stat->blksize = buf.st_blksize;
    stat->blocks = buf.st_blocks;
  }
#else
  
#endif
  return ret;
}
