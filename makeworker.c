/**@file makeobj.c
 * @brief make all c/c++ source file to object file
 * @note
 * @email 25452483@qq.com
 * @history 2016-8-28 ZRiemann found
 */
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>

#ifndef NULL
#defnie NULL 0
#endif
typedef int (*cbfile)(const char*, const struct stat*, int);
static void useage();
static int cbf(const char *pathname, const struct stat *statptr, int type);
static int zftw(char*, cbfile);
static int dopath(cbfile);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
static const char* destdir;
static const char* suffix;
static const char* cc;
static const char* flags = "";
int main(int argc, char* argv[]){
  DIR *dir;
  int ret;
  if(argc < 5) useage();
  destdir = argv[2];
  suffix = argv[3];
  cc = argv[4];
  if(argc == 6){
    flags = argv[5];
  }
  ret = zftw(argv[1], cbf);
  /*
  ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
  if(ntot == 0)ntot = 1;
  printf("regular files =\t %7ld, %5.2f %%\n", nreg, nreg*100.0/ntot);
  printf("directories =\t %7ld, %5.2f %%\n", ndir, ndir*100.0/ntot);
  printf("block special =\t %7ld, %5.2f %%\n", nblk, nblk*100.0/ntot);
  printf("char special =\t %7ld, %5.2f %%\n", nchr, nchr*100.0/ntot);
  printf("FIFOs =\t %7ld, %5.2f %%\n", nfifo, nfifo*100.0/ntot);
  printf("symbolic links =\t %7ld, %5.2f %%\n", nslink, nslink*100.0/ntot);
  printf("sockets =\t %7ld, %5.2f %%\n", nsock, nsock*100.0/ntot);
  */
  exit(ret);
}

static void useage(){
  printf("makeobj [src_dir] [dest_dir] [.c|.cxx|.cpp] [gcc|g++|cc] [flags]\n");
  exit(0);
}

/* Descend through the hierarchy, starting at "pathname",
 * The caller's fun() is called for every file.
 */
#define FTW_F 1 /* file other then directory*/
#define FTW_D 2 /* directory */
#define FTW_DNR 3 /* directory that can't be read */
#define FTW_NS 4 /* file that we can't stat */
static char fullpath[512]; /* contains full pathname for every file */

static int zftw(char *pathname, cbfile cbfil){
  strcpy(fullpath, pathname);
  return(dopath(cbfil));
}

static int dopath(cbfile cbfil){
  struct stat statbuf;
  struct dirent *dirp;
  DIR *dp;
  int ret, n;
  if(lstat(fullpath, &statbuf) < 0)return(cbfil(fullpath, &statbuf, FTW_NS));
  if(S_ISDIR(statbuf.st_mode)==0)return(cbfil(fullpath, &statbuf, FTW_F));
  if((ret = cbfil(fullpath, &statbuf, FTW_D))!=0)return(ret);
  n = strlen(fullpath);
  fullpath[n++]= '/';
  fullpath[n] = 0;
  if((dp = opendir(fullpath)) == NULL)
    return(cbfil(fullpath, &statbuf, FTW_DNR));
  while((dirp = readdir(dp)) != NULL){
    if(strcmp(dirp->d_name,".")==0 || strcmp(dirp->d_name,"..")==0)continue;
    strcpy(&fullpath[n], dirp->d_name);
    if((ret = dopath(cbfil)) != 0)break;
  }
  fullpath[n-1] = 0;
  if(closedir(dp)<0)printf("can't close directory %s", fullpath);
  return(ret);
}

int check_modify_time(const struct stat *srcst, const char *destfile){
  int ret = 0;
  static struct stat destst;
  if(lstat(destfile,&destst) < 0){
    //cbf(destfile, &destst, FTW_NS);
    return 0; // file not exist.
  }
  if(srcst->st_mtime <= destst.st_mtime){
    ret = -2; // not neet compile
  }
  return(ret);
}
static void docmd(const char *pathname, const struct stat *statptr)
{
  static char cmdbuf[1024];
  static char filename[64];
  int offset;
  char *find;

  find = strstr(pathname, suffix);
  if(find && strcmp(find, suffix) == 0){
    snprintf(filename,64,"%s", strrchr(pathname, '/')+1);
    offset = strlen(filename)- (strlen(suffix)-1);
    filename[offset]='o';
    filename[offset+1] = 0;
    sprintf(cmdbuf,"%s/%s", destdir, filename); 
    if(0 == check_modify_time(statptr, cmdbuf)){
      snprintf(cmdbuf,1024,"%s %s -c %s -o %s/%s", cc, flags, pathname, destdir,filename);
      printf("%s\n",cmdbuf);
      system(cmdbuf);
    }
  }
}
static int cbf(const char *pathname, const struct stat *statptr, int type){
  switch(type){
  case FTW_F:
    switch(statptr->st_mode & S_IFMT){
    case S_IFREG:
      docmd(pathname, statptr);
      nreg++;
      break;
    case S_IFBLK:nblk++;break;
    case S_IFCHR:nchr++;break;
    case S_IFIFO:nfifo++;break;
    case S_IFLNK:nslink++;break;
    case S_IFSOCK:nsock++;break;
    case S_IFDIR:printf("for S_IFDIR for %s", pathname);
    }
    break;
  case FTW_D:
    ndir++;
    break;
  case FTW_DNR:
    printf("can't read directory %s", pathname);
    break;
  case FTW_NS:
    printf("stat error for %s", pathname);
    break;
  default:
    printf("unknown type %d for pathname %s", type, pathname);
  }
  return(0);
}
