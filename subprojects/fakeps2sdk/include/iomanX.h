#pragma once

#include "irx.h"

#define O_RDONLY 0x0001
#define O_WRONLY 0x0002
#define O_RDWR 0x0003
#define O_DIROPEN 0x0008 // Internal use for dopen
#define O_NBLOCK 0x0010
#define O_APPEND 0x0100
#define O_CREAT 0x0200
#define O_TRUNC 0x0400
#define O_EXCL 0x0800
#define O_NOWAIT 0x8000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* Device drivers.  */

/* Device types.  */
#define IOP_DT_CHAR 0x01
#define IOP_DT_CONS 0x02
#define IOP_DT_BLOCK 0x04
#define IOP_DT_RAW 0x08
#define IOP_DT_FS 0x10
#ifndef IOMAN_NO_EXTENDED
#define IOP_DT_FSEXT 0x10000000 /* Supports calls after chstat().  */
#endif

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

/* File objects passed to driver operations.  */
typedef struct _iop_file
{
    s32 mode;                   /* File open mode.  */
    s32 unit;                   /* HW device unit number.  */
    struct _iop_device *device; /* Device driver.  */
    void *privdata;             /* The device driver can use this however it
				   wants.  */
} iop_file_t;

typedef struct _iop_device
{
    const char *name;
    u32 type;
    u32 version; /* Not so sure about this one.  */
    const char *desc;
    struct _iop_device_ops *ops;
} iop_device_t;


#define open iomanx_open
#define close iomanx_close
#define read iomanx_read
#define write iomanx_write
#define lseek iomanx_lseek
#define ioctl iomanx_ioctl
#define remove iomanx_remove
#define mkdir iomanx_mkdir
#define rmdir iomanx_rmdir
#define dopen iomanx_dopen
#define dclose iomanx_dclose
#define dread iomanx_dread
#define getstat iomanx_getstat
#define chstat iomanx_chstat

#define format iomanx_format
#define rename iomanx_rename
#define chdir iomanx_chdir
#define sync iomanx_sync
#define mount iomanx_mount
#define umount iomanx_umount
#define lseek64 iomanx_lseek64
#define devctl iomanx_devctl
#define symlink iomanx_symlink
#define readlink iomanx_readlink
#define ioctl2 iomanx_ioctl2


typedef struct _iop_device_ops
{
    int (*init)(iop_device_t *);
    int (*deinit)(iop_device_t *);
    int (*format)(iop_file_t *, const char *, const char *, void *, size_t);
    int (*open)(iop_file_t *, const char *, int, int);
    int (*close)(iop_file_t *);
    int (*read)(iop_file_t *, void *, int);
    int (*write)(iop_file_t *, void *, int);
    int (*lseek)(iop_file_t *, int, int);
    int (*ioctl)(iop_file_t *, int, void *);
    int (*remove)(iop_file_t *, const char *);
    int (*mkdir)(iop_file_t *, const char *, int);
    int (*rmdir)(iop_file_t *, const char *);
    int (*dopen)(iop_file_t *, const char *);
    int (*dclose)(iop_file_t *);
    int (*dread)(iop_file_t *, iox_dirent_t *);
    int (*getstat)(iop_file_t *, const char *, iox_stat_t *);
    int (*chstat)(iop_file_t *, const char *, iox_stat_t *, unsigned int);

#ifndef IOMAN_NO_EXTENDED
    /* Extended ops start here.  */
    int (*rename)(iop_file_t *, const char *, const char *);
    int (*chdir)(iop_file_t *, const char *);
    int (*sync)(iop_file_t *, const char *, int);
    int (*mount)(iop_file_t *, const char *, const char *, int, void *, int);
    int (*umount)(iop_file_t *, const char *);
    s64 (*lseek64)(iop_file_t *, s64, int);
    int (*devctl)(iop_file_t *, const char *, int, void *, size_t, void *, size_t);
    int (*symlink)(iop_file_t *, const char *, const char *);
    int (*readlink)(iop_file_t *, const char *, char *, unsigned int);
    int (*ioctl2)(iop_file_t *, int, void *, size_t, void *, size_t);
#endif /* IOMAN_NO_EXTENDED */
} iop_device_ops_t;

iop_device_t **GetDeviceList(void);

int open(const char *name, int flags, ...);
int close(int fd);
int read(int fd, void *ptr, int size);
int write(int fd, void *ptr, int size);
int lseek(int fd, int offset, int mode);

int ioctl(int fd, int cmd, void *param);
int remove(const char *name);

int mkdir(const char *path, int mode);
int rmdir(const char *path);
int dopen(const char *path);
int dclose(int fd);
int dread(int fd, iox_dirent_t *buf);

int getstat(const char *name, iox_stat_t *stat);
int chstat(const char *name, iox_stat_t *stat, unsigned int statmask);

/** This can take take more than one form.  */
int format(const char *dev, const char *blockdev, void *arg, int arglen);

/* The newer calls - these are NOT supported by the older IOMAN.  */
int rename(const char *old, const char *new);
int chdir(const char *name);
int sync(const char *dev, int flag);
int mount(const char *fsname, const char *devname, int flag, void *arg, int arglen);
int umount(const char *fsname);
s64 lseek64(int fd, s64 offset, int whence);
int devctl(const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);
int symlink(const char *old, const char *new);
int readlink(const char *path, char *buf, unsigned int buflen);
int ioctl2(int fd, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen);

int AddDrv(iop_device_t *device);
int DelDrv(const char *name);

void StdioInit(int mode);


// Access flags for filesystem mount
#define FIO_MT_RDWR 0x00
#define FIO_MT_RDONLY 0x01

#define IOMANX_SEEK_SET 0
#define IOMANX_SEEK_CUR 1
#define IOMANX_SEEK_END 2

// File mode flags (for mode in io_stat_t)
#define FIO_SO_IFMT 0x0038  // Format mask
#define FIO_SO_IFLNK 0x0008 // Symbolic link
#define FIO_SO_IFREG 0x0010 // Regular file
#define FIO_SO_IFDIR 0x0020 // Directory

#define FIO_SO_IROTH 0x0004 // read
#define FIO_SO_IWOTH 0x0002 // write
#define FIO_SO_IXOTH 0x0001 // execute

// File mode checking macros
#define FIO_SO_ISLNK(m) (((m)&FIO_SO_IFMT) == FIO_SO_IFLNK)
#define FIO_SO_ISREG(m) (((m)&FIO_SO_IFMT) == FIO_SO_IFREG)
#define FIO_SO_ISDIR(m) (((m)&FIO_SO_IFMT) == FIO_SO_IFDIR)
