#pragma once

#include "pfsd_common.h"
#include "pfslib_compat.h"
#include <stddef.h>

extern int scepfsdInit(void);
extern void scepfsdExit(void);
extern int scepfsdOpen(const char *name, int flags, int mode);
extern int scepfsdLseek(int fd, int offset, int whence);
extern int64_t scepfsdLseek64(int fd, int64_t offset, int whence);
extern int scepfsdRead(int fd, void *ptr, size_t size);
extern int scepfsdWrite(int fd, void *ptr, size_t size);
extern int scepfsdClose(int fd);
extern int scepfsdDopen(const char *name);
extern int scepfsdDread(int fd, iox_dirent_t *iox_dirent);
extern int scepfsdDclose(int fd);
extern int scepfsdChstat(const char *name, iox_stat_t *stat, int mask);
extern int scepfsdGetstat(const char *name, iox_stat_t *stat);
extern int scepfsdMkdir(const char *name, int mode);
extern int scepfsdRmdir(const char *name);
extern int scepfsdChdir(const char *name);
extern int scepfsdRemove(const char *name);
extern int scepfsdRename(const char *old, const char *new_);
extern int scepfsdIoctl(int fd, int cmd, void *arg);
extern int scepfsdIoctl2(int fd, int command, void *arg, size_t arglen, void *buf, size_t buflen);
extern int scepfsdSymlink(const char *old, const char *new_);
extern int scepfsdReadlink(const char *name, char *buf, size_t buflen);
extern int scepfsdSync(const char *dev, int flag);
extern int scepfsdFormat(const char *dev, const char *blockdev, void *arg, size_t arglen);
extern int scepfsdMount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen);
extern int scepfsdUmount(const char *fsname);
extern int scepfsdDevctl(const char *name, int cmd, const void *arg, size_t arglen, void *buf, size_t buflen);
extern int scepfsdSetReadAhead(int enabled);
extern int scepfsdGetReadAhead(void);
extern int scepfsdGetMountPoint(const char *in_oldmap, char *out_newmap);
