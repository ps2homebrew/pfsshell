#pragma once
#include <stdlib.h>
#include <stdint.h>

/*
 * most constants were prefixed with `IOMANX_' because
 * of name clashes against OS' APIs;
 *
 * that header is absolutely sufficient if you wish to do iomanX I/O;
 *
 * including ps2sdk headers could possibly lead to hard to track
 * problems and data loss, because of different values
 * for common constants, such as O_RDONLY, etc.
 * (imagine a call, such as open ("foo", O_RDONLY) which OS reads as
 *  open ("foo", O_WRONLY | O_CREAT) )
 */

/* those three would return 0 on success */
extern int _init_apa(int argc, char *argv[]);
extern int _init_pfs(int argc, char *argv[]);
extern int _init_hdlfs(int argc, char *argv[]);

#define IOMANX_O_RDONLY 0x0001
#define IOMANX_O_WRONLY 0x0002
#define IOMANX_O_RDWR 0x0003
#define IOMANX_O_DIROPEN 0x0008 // Internal use for dopen
#define IOMANX_O_NBLOCK 0x0010
#define IOMANX_O_APPEND 0x0100
#define IOMANX_O_CREAT 0x0200
#define IOMANX_O_TRUNC 0x0400
#define IOMANX_O_EXCL 0x0800
#define IOMANX_O_NOWAIT 0x8000

// Access flags for filesystem mount
#define FIO_MT_RDWR 0x00
#define FIO_MT_RDONLY 0x01

#define IOMANX_SEEK_SET 0
#define IOMANX_SEEK_CUR 1
#define IOMANX_SEEK_END 2

// Flags for chstat 'statmask'
#define FIO_CST_MODE 0x0001
#define FIO_CST_ATTR 0x0002
#define FIO_CST_SIZE 0x0004
#define FIO_CST_CT 0x0008
#define FIO_CST_AT 0x0010
#define FIO_CST_MT 0x0020
#define FIO_CST_PRVT 0x0040

// File mode flags
#define FIO_S_IFMT 0xF000  // Format mask
#define FIO_S_IFLNK 0x4000 // Symbolic link
#define FIO_S_IFREG 0x2000 // Regular file
#define FIO_S_IFDIR 0x1000 // Directory

// Access rights
#define FIO_S_ISUID 0x0800 // SUID
#define FIO_S_ISGID 0x0400 // SGID
#define FIO_S_ISVTX 0x0200 // Sticky bit

#define FIO_S_IRWXU 0x01C0 // User access rights mask
#define FIO_S_IRUSR 0x0100 // read
#define FIO_S_IWUSR 0x0080 // write
#define FIO_S_IXUSR 0x0040 // execute

#define FIO_S_IRWXG 0x0038 // Group access rights mask
#define FIO_S_IRGRP 0x0020 // read
#define FIO_S_IWGRP 0x0010 // write
#define FIO_S_IXGRP 0x0008 // execute

#define FIO_S_IRWXO 0x0007 // Others access rights mask
#define FIO_S_IROTH 0x0004 // read
#define FIO_S_IWOTH 0x0002 // write
#define FIO_S_IXOTH 0x0001 // execute

// File mode checking macros
#define FIO_S_ISLNK(m) (((m)&FIO_S_IFMT) == FIO_S_IFLNK)
#define FIO_S_ISREG(m) (((m)&FIO_S_IFMT) == FIO_S_IFREG)
#define FIO_S_ISDIR(m) (((m)&FIO_S_IFMT) == FIO_S_IFDIR)

/* File attributes that are retrieved using the getstat and dread calls, and
   set using chstat.  */

/* ps2_hdd.h: Date/time descriptor used in on-disk partition header */
typedef struct ps2fs_datetime_type
{
    uint8_t unused;
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} ps2fs_datetime_t;

/* The following structures are only supported by iomanX.  */
#ifndef iomanX_struct
#define iomanX_struct
typedef struct
{
    /*00*/ unsigned int mode;
    /*04*/ unsigned int attr;
    /*08*/ unsigned int size;
    /*0c*/ unsigned char ctime[8];
    /*14*/ unsigned char atime[8];
    /*1c*/ unsigned char mtime[8];
    /*24*/ unsigned int hisize;
    /*28*/ unsigned int private_0; //Number of subs (main) / subpart number (sub)
    /*2c*/ unsigned int private_1;
    /*30*/ unsigned int private_2;
    /*34*/ unsigned int private_3;
    /*38*/ unsigned int private_4;
    /*3c*/ unsigned int private_5; /* Sector start.  */
} iox_stat_t;

typedef struct
{
    iox_stat_t stat;
    char name[256];
    unsigned int unknown;
} iox_dirent_t;
#endif

/* open() takes an optional mode argument.  */
int iomanx_open(const char *name, int flags, ...);
int iomanx_close(int fd);
int iomanx_read(int fd, void *ptr, size_t size);
int iomanx_write(int fd, void *ptr, size_t size);
int iomanx_lseek(int fd, int offset, int mode);

int iomanx_ioctl(int fd, unsigned long cmd, void *param);
int iomanx_remove(const char *name);

/* mkdir() takes an optional mode argument.  */
int iomanx_mkdir(const char *path, ...);
int iomanx_rmdir(const char *path);
int iomanx_dopen(const char *path);
int iomanx_dclose(int fd);
int iomanx_dread(int fd, void *buf);

int iomanx_getstat(const char *name, void *stat);
int iomanx_chstat(const char *name, void *stat, unsigned int statmask);

/* This can take take more than one form.  */
int iomanx_format(const char *dev, const char *blockdev, void *arg, size_t arglen);

/* The newer calls - these are NOT supported by the older IOMAN.  */
int iomanx_rename(const char *old, const char *new);
int iomanx_chdir(const char *name);
int iomanx_sync(const char *dev, int flag);
int iomanx_mount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen);
int iomanx_umount(const char *fsname);
int iomanx_lseek64(int fd, long long offset, int whence);
int iomanx_devctl(const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
int iomanx_symlink(const char *old, const char *new);
int iomanx_readlink(const char *path, char *buf, size_t buflen);
int iomanx_ioctl2(int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);

//const char* strerror (int err);
