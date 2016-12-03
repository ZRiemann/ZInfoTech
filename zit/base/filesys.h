#ifndef _ZBASE_FILE_SYS_H_
#define _ZBASE_FILE_SYS_H_

#include "platform.h"
#include <zit/base/type.h>
#include <time.h>

ZC_BEGIN

#ifdef ZSYS_POSIX
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
typedef int zfd_t;
#define INVALID_FD (-1)
#else // windows

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 4
#define O_CREAT 8
#define O_TRUNC 0x10
#define O_APPEND 0x20

typedef HANDLE zfd_t;
#define INVALID_FD INVALID_HANDLE_VALUE

#endif

#define ZFMODE_REGULAR 0 // regular file
#define ZFMODE_DIR 1 // directory
#define ZFMODE_CHAR 2 // character special
#define ZFMODE_BLOCK 3 // block special
#define ZFMODE_FIFO 4 // fifo
#define ZFMODE_LNK 5 // symbolic link
#define ZFMODE_SOCK 6 // socket
#define ZFMODE_UNKNOWN 7
typedef struct zfile_stat_s{
  int mode; // ZFMODE_*
  int ino;
  int dev;
  int rdev;
  int nlink;
  int uid;
  int gid;
  int size;
  struct tm atime;
  struct tm mtime;
  struct tm ctime;
  int blksize;
  int blocks;
}zfstat_t;

// file io api
/**@fn int zopen(const char *fname, int flag, int mode)
 * @brief open/create a file or device
 * @param const char *fname [in] file name pointer
 * @param int flag [in] 
 *        O_RDONLY/O_WRONLY/O_REWR main flag
 *        O_APPEND/O_TRUNC/O_CREATE/O_EXCL sub flag
 *        O_EXEC/O_SEARCH/O_CLOEXEC/O_DIRECTORY/O_DSYNC/O_RSYNC
 *        O_NOTTY/O_NOFLLOW/O_NONBLOCK/O_SYNC/O_TTY_INIT
 * @param int mode [in] access mode (O_CREATE)
 *        S_IRUSR/S_IWUSR/S_IXUSR/S_IRWXU
 *        S_IRGRP/S_IWGRP/S_IXGRP/S_IRWXG
 *        S_IROTH/S_IWOTH/S_IXOTH/S_IRWXO
 */
ZAPI zfd_t zfopen(const char *fname, int flag, int mode);
ZAPI int zfclose(zfd_t fd);
ZAPI int zfread(zfd_t fd, void *buf, int nbytes);
ZAPI int zfwrite(zfd_t fd, const void *buf, int nbytes);
//ZAPI int zflseek();
//ZAPI int zfdup();
//ZAPI int zfcntl();
//ZAPI int zsync();
//ZAPI int zfioctl();

// directory api
#define FTW_F 1 // file other then directory
#define FTW_D 2 // directory
#define FTW_DNR 3 // directory that can't be read
#define FTW_NS 4 // file that we can't stat

ZAPI int zmkdir(const char *dir, int mode);
ZAPI int zrmdir(const char *dir);
ZAPI int zrmfile(const char *fname);
//ZAPI void *zopendir(const char *dir);
//ZAPI void *zreaddir(void *handle);
typedef int cbftwf(zfstat_t *stat);
ZAPI int zfstat(const char *pathname, zfstat_t *stat);
ZAPI int zftw(const char *dir, zfstat_t stat, int type);

ZC_END
#endif
