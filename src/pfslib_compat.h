#pragma once
#include "iomanX_port.h"

extern int scepfsInit(void);
extern void scepfsExit(void);
extern int scepfsSetReadAhead(int enabled);
extern int scepfsGetReadAhead(void);


static inline int scepfsOpen(const char *name, int flags, int mode)
{
    return iomanX_open(name, flags, mode);
}

static inline int scepfsLseek(int fd, int offset, int whence)
{
    return iomanX_lseek(fd, offset, whence);
}

static inline int64_t scepfsLseek64(int fd, int64_t offset, int whence)
{
    return iomanX_lseek64(fd, offset, whence);
}

static inline int scepfsRead(int fd, void *ptr, size_t size)
{
    return iomanX_read(fd, ptr, size);
}

static inline int scepfsWrite(int fd, void *ptr, size_t size)
{
    return iomanX_write(fd, ptr, size);
}

static inline int scepfsClose(int fd)
{
    return iomanX_close(fd);
}

static inline int scepfsDopen(const char *name)
{
    return iomanX_dopen(name);
}

static inline int scepfsDread(int fd, iox_dirent_t *iox_dirent)
{
    return iomanX_dread(fd, iox_dirent);
}

static inline int scepfsDclose(int fd)
{
    return iomanX_close(fd);
}

static inline int scepfsChstat(const char *name, iox_stat_t *stat, int mask)
{
    return iomanX_chstat(name, stat, mask);
}

static inline int scepfsGetstat(const char *name, iox_stat_t *stat)
{
    return iomanX_getstat(name, stat);
}

static inline int scepfsMkdir(const char *name, int mode)
{
    return iomanX_mkdir(name, mode);
}

static inline int scepfsRmdir(const char *name)
{
    return iomanX_rmdir(name);
}

static inline int scepfsChdir(const char *name)
{
    return iomanX_chdir(name);
}

static inline int scepfsRemove(const char *name)
{
    return iomanX_remove(name);
}

static inline int scepfsRename(const char *old, const char *new_)
{
    return iomanX_rename(old, new_);
}

static inline int scepfsIoctl(int fd, int cmd, void *arg)
{
    return iomanX_ioctl(fd, cmd, arg);
}

static inline int scepfsIoctl2(int fd, int command, void *arg, size_t arglen, void *buf, size_t buflen)
{
    return iomanX_ioctl2(fd, command, arg, arglen, buf, buflen);
}

static inline int scepfsSymlink(const char *old, const char *new_)
{
    return iomanX_symlink(old, new_);
}

static inline int scepfsReadlink(const char *name, char *buf, size_t buflen)
{
    return iomanX_readlink(name, buf, buflen);
}

static inline int scepfsSync(const char *dev, int flag)
{
    return iomanX_sync(dev, flag);
}

static inline int scepfsFormat(const char *dev, const char *blockdev, void *arg, size_t arglen)
{
    return iomanX_format(dev, blockdev, arg, arglen);
}

static inline int scepfsMount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen)
{
    return iomanX_mount(fsname, devname, flag, arg, arglen);
}

static inline int scepfsUmount(const char *fsname)
{
    return iomanX_umount(fsname);
}

static inline int scepfsDevctl(const char *name, int cmd, void *arg, size_t arglen, void *buf, size_t buflen)
{
    return iomanX_devctl(name, cmd, arg, arglen, buf, buflen);
}
