#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "iomanX_port.h"

static int host_translator_op_init(iop_device_t *f)
{
	(void)f;

	return -48;
}

static int host_translator_op_exit(iop_device_t *f)
{
	(void)f;

	return -48;
}

static int host_translator_op_format(iop_file_t *f, const char *dev, const char *blockdev, void *arg, int arglen)
{
	(void)f;
	(void)dev;
	(void)blockdev;
	(void)arg;
	(void)arglen;

	return -48;
}

static int host_translator_op_open(iop_file_t *f, const char *name, int flags, int mode)
{
	int translated_flags = 0;
    if ((flags & FIO_O_RDWR) != 0) {
        flags |= O_RDWR;
    }
    else if ((flags & FIO_O_RDONLY) != 0) {
        flags |= O_RDONLY;
    }
    else if ((flags & FIO_O_WRONLY) != 0) {
        flags |= O_WRONLY;
    }
#ifndef _WIN32
    if ((flags & FIO_O_NBLOCK) != 0) {
        flags |= O_NONBLOCK;
    }
#endif
    if ((flags & FIO_O_APPEND) != 0) {
        flags |= O_APPEND;
    }
    flags |= O_CREAT;
    if ((flags & FIO_O_TRUNC) != 0) {
        flags |= O_TRUNC;
    }
    if ((flags & FIO_O_EXCL) != 0) {
        flags |= O_EXCL;
    }

	int fh = open(name, flags, mode);
	if (fh < 0)
	{
		// TODO: translate errno
		return -errno;
	}
	f->privdata = (void *)(uintptr_t)fh;
	return 0;
}

static int host_translator_op_close(iop_file_t *f)
{
	int res = close((int)(uintptr_t)f->privdata);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

	return 0;
}

static int host_translator_op_read(iop_file_t *f, void *ptr, int size)
{
	int res = read((int)(uintptr_t)f->privdata, ptr, size);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

	return res;
}

static int host_translator_op_write(iop_file_t *f, void *ptr, int size)
{
	int res = write((int)(uintptr_t)f->privdata, (const void *)ptr, size);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return res;
}

static int host_translator_op_lseek(iop_file_t *f, int offset, int mode)
{
	// TODO: translate mode
	int res = lseek((int)(uintptr_t)f->privdata, offset, mode);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return res;
}

static int host_translator_op_ioctl(iop_file_t *f, int cmd, void *param)
{
	(void)f;
	(void)cmd;
	(void)param;

	// Intentionally not handled.

    return -48;
}

static int host_translator_op_remove(iop_file_t *f, const char *name)
{
    (void)f;

	// TODO: should we also handle directories?
	int res = unlink(name);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return 0;
}

static int host_translator_op_mkdir(iop_file_t *f, const char *path, int mode)
{
    (void)f;

#ifndef _WIN32
	int res = mkdir(path, mode);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}
#endif

    return 0;
}

static int host_translator_op_rmdir(iop_file_t *f, const char *path)
{
    (void)f;

	int res = rmdir(path);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return 0;
}

static int host_translator_op_dopen(iop_file_t *f, const char *path)
{
    DIR *res = opendir(path);
    if (res == NULL)
    {
		// TODO: translate errno
		return -errno;
    }
    f->privdata = (void *)res;

    return 0;
}

static int host_translator_op_dclose(iop_file_t *f)
{
    if (f->privdata == NULL)
    {
    	return -EBADF;
    }

    int res = closedir((DIR *)f->privdata);
    if (res < 0)
    {
    	// TODO: translate errno
    	return -errno;
    }

    return 0;
}

static int host_translator_op_dread(iop_file_t *f, iox_dirent_t *buf)
{
    if (f->privdata == NULL)
    {
    	return -EBADF;
    }

    errno = 0;
    struct dirent *d = readdir((DIR *)f->privdata);
    if (d == NULL)
    {
    	if (errno != 0)
    	{
    		return -errno;
    	}
    	else
    	{
    		return -1;
    	}
    }

    memset(buf, 0, sizeof(*buf));
    int name_len = strlen(d->d_name);
    if (name_len > sizeof(buf->name))
    {
    	name_len = sizeof(buf->name);
    }
    memcpy(buf->name, d->d_name, name_len);

    // TODO: fill in the rest of the iox_dirent_t structure

    return 0;
}

static void convert_mode_to_posix(mode_t *posix_mode, const unsigned int *iomanx_mode)
{
    *posix_mode = 0;
    if (FIO_S_ISDIR(*iomanx_mode)) {
        *posix_mode |= S_IFDIR;
    }
    if (FIO_S_ISREG(*iomanx_mode)) {
        *posix_mode |= S_IFREG;
    }
#ifndef _WIN32
    if (FIO_S_ISLNK(*iomanx_mode)) {
        *posix_mode |= S_IFLNK;
    }
#endif
#if 0
    if (*iomanx_mode & FIO_S_IRUSR) {
        *posix_mode |= S_IRUSR;
    }
    if (*iomanx_mode & FIO_S_IWUSR) {
        *posix_mode |= S_IWUSR;
    }
    if (*iomanx_mode & FIO_S_IXUSR) {
        *posix_mode |= S_IXUSR;
    }
    if (*iomanx_mode & FIO_S_IRGRP) {
        *posix_mode |= S_IRGRP;
    }
    if (*iomanx_mode & FIO_S_IWGRP) {
        *posix_mode |= S_IWGRP;
    }
    if (*iomanx_mode & FIO_S_IXGRP) {
        *posix_mode |= S_IXGRP;
    }
    if (*iomanx_mode & FIO_S_IROTH) {
        *posix_mode |= S_IROTH;
    }
    if (*iomanx_mode & FIO_S_IWOTH) {
        *posix_mode |= S_IWOTH;
    }
    if (*iomanx_mode & FIO_S_IXOTH) {
        *posix_mode |= S_IXOTH;
    }
#else
    if (*iomanx_mode & (FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH)) {
        *posix_mode |= S_IRUSR | S_IRGRP | S_IROTH;
    }
    if (*iomanx_mode & (FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH)) {
        *posix_mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    }
    if (*iomanx_mode & (FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH)) {
        *posix_mode |= S_IXUSR | S_IXGRP | S_IXOTH;
    }
#endif
#ifndef _WIN32
    if (*iomanx_mode & FIO_S_ISUID) {
        *posix_mode |= S_ISUID;
    }
    if (*iomanx_mode & FIO_S_ISGID) {
        *posix_mode |= S_ISGID;
    }
    if (*iomanx_mode & FIO_S_ISVTX) {
        *posix_mode |= S_ISVTX;
    }
#endif
}

static void convert_mode_to_iomanx(unsigned int *iomanx_mode, const mode_t *posix_mode)
{
    *iomanx_mode = 0;
    if (S_ISDIR(*posix_mode)) {
        *iomanx_mode |= FIO_S_IFDIR;
    }
    if (S_ISREG(*posix_mode)) {
        *iomanx_mode |= FIO_S_IFREG;
    }
#ifndef _WIN32
    if (S_ISLNK(*posix_mode)) {
        *iomanx_mode |= FIO_S_IFLNK;
    }
#endif
#if 0
    if (*posix_mode & S_IRUSR) {
        *iomanx_mode |= FIO_S_IRUSR;
    }
    if (*posix_mode & S_IWUSR) {
        *iomanx_mode |= FIO_S_IWUSR;
    }
    if (*posix_mode & S_IXUSR) {
        *iomanx_mode |= FIO_S_IXUSR;
    }
    if (*posix_mode & S_IRGRP) {
        *iomanx_mode |= FIO_S_IRGRP;
    }
    if (*posix_mode & S_IWGRP) {
        *iomanx_mode |= FIO_S_IWGRP;
    }
    if (*posix_mode & S_IXGRP) {
        *iomanx_mode |= FIO_S_IXGRP;
    }
    if (*posix_mode & S_IROTH) {
        *iomanx_mode |= FIO_S_IROTH;
    }
    if (*posix_mode & S_IWOTH) {
        *iomanx_mode |= FIO_S_IWOTH;
    }
    if (*posix_mode & S_IXOTH) {
        *iomanx_mode |= FIO_S_IXOTH;
    }
#else
    if (*posix_mode & (S_IRUSR | S_IRGRP | S_IROTH)) {
        *iomanx_mode |= FIO_S_IRUSR | FIO_S_IRGRP | FIO_S_IROTH;
    }
    if (*posix_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) {
        *iomanx_mode |= FIO_S_IWUSR | FIO_S_IWGRP | FIO_S_IWOTH;
    }
    if (*posix_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        *iomanx_mode |= FIO_S_IXUSR | FIO_S_IXGRP | FIO_S_IXOTH;
    }
#endif
#ifndef _WIN32
    if (*posix_mode & S_ISUID) {
        *iomanx_mode |= FIO_S_ISUID;
    }
    if (*posix_mode & S_ISGID) {
        *iomanx_mode |= FIO_S_ISGID;
    }
    if (*posix_mode & S_ISVTX) {
        *iomanx_mode |= FIO_S_ISVTX;
    }
#endif
}

static void convert_time_to_iomanx(unsigned char *iomanx_time, const time_t *posix_time)
{
    struct tm timeinfo;
    time_t rawtime = *posix_time;
    // convert JST->UTC
    rawtime -= (9 * 60 * 60);
#ifndef _WIN32
    gmtime_r(&rawtime, &timeinfo);
#else
    gmtime_s(&timeinfo, &rawtime);
#endif
    iomanx_time[0] = 0;
    iomanx_time[1] = timeinfo.tm_sec;
    iomanx_time[2] = timeinfo.tm_min;
    iomanx_time[3] = timeinfo.tm_hour;
    iomanx_time[4] = timeinfo.tm_mday;
    iomanx_time[5] = timeinfo.tm_mon + 1;              // month 1 (January) is 0
    iomanx_time[6] = (timeinfo.tm_year + 1900) & 0xff; // year 1900 is 0
    iomanx_time[7] = ((timeinfo.tm_year + 1900) >> 8) & 0xff;
}

static void convert_stat_to_iomanx(iox_stat_t *iomanx_stat, const struct stat *posix_stat)
{
    memset(iomanx_stat, 0, sizeof(*iomanx_stat));
    iomanx_stat->size = posix_stat->st_size & 0xffffffff;
    if (sizeof(posix_stat->st_size) > 4) {
        iomanx_stat->hisize = ((off_t)posix_stat->st_size >> 32) & 0xffffffff;
    }
    convert_mode_to_iomanx(&(iomanx_stat->mode), &(posix_stat->st_mode));
#if 0
    posix_stat->st_attr = iomanx_stat->attr;
#endif
#ifndef _WIN32
    convert_time_to_iomanx(iomanx_stat->ctime, &(posix_stat->st_ctime));
    convert_time_to_iomanx(iomanx_stat->atime, &(posix_stat->st_atime));
    convert_time_to_iomanx(iomanx_stat->mtime, &(posix_stat->st_mtime));
#else
#if 0
    convert_time_to_iomanx(iomanx_stat->ctime, &(posix_stat->st_ctim));
    convert_time_to_iomanx(iomanx_stat->atime, &(posix_stat->st_atim));
    convert_time_to_iomanx(iomanx_stat->mtime, &(posix_stat->st_mtim));
#endif
#endif
#if 0
    posix_stat->st_uid = iomanx_stat->private_0;
    posix_stat->st_gid = iomanx_stat->private_1;
#endif
}

static int host_translator_op_getstat(iop_file_t *f, const char *name, iox_stat_t *iomanx_stat)
{
	struct stat posix_stat;

    (void)f;

	int res = stat(name, &posix_stat);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

	convert_stat_to_iomanx(iomanx_stat, &posix_stat);

    return 0;
}

static int host_translator_op_chstat(iop_file_t *f, const char *name, iox_stat_t *iomanx_stat, unsigned int statmask)
{
    (void)f;

    if (iomanx_stat == NULL)
    {
    	return -EBADF;
    }

    if ((statmask & FIO_CST_MODE) != 0)
    {
    	mode_t posix_mode = 0;
    	convert_mode_to_posix(&posix_mode, &(iomanx_stat->mode));
    	int res = chmod(name, posix_mode);
		if (res < 0)
		{
			// TODO: translate errno
			return -errno;
		}
    }

    if ((statmask & FIO_CST_AT) != 0)
    {
    	// TODO: get stat from stat, then set time using utimes
    }

    if ((statmask & FIO_CST_MT) != 0)
    {
    	// TODO: get stat from stat, then set time using utimes
    }

    return 0;
}

static int host_translator_op_rename(iop_file_t *f, const char *old, const char *new_1)
{
    (void)f;

	int res = rename(old, new_1);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return 0;
}

static int host_translator_op_chdir(iop_file_t *f, const char *name)
{
    (void)f;

	int res = chdir(name);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return 0;
}

static int host_translator_op_sync(iop_file_t *f, const char *dev, int flag)
{
    (void)f;
    (void)dev;
    (void)flag;

#ifndef _WIN32
	sync();
#endif

    return 0;
}

static int host_translator_op_mount(iop_file_t *f, const char *fsname, const char *devname, int flag, void *arg, int arglen)
{
    (void)f;
    (void)fsname;
    (void)devname;
    (void)flag;
    (void)arg;
    (void)arglen;

    // Intentionally not handled.

    return -48;
}

static int host_translator_op_umount(iop_file_t *f, const char *fsname)
{
    (void)f;
    (void)fsname;

    // Intentionally not handled.

    return -48;
}

static int64_t host_translator_op_lseek64(iop_file_t *f, int64_t offset, int whence)
{
	// TODO: translate whence
	int res = lseek((int)(uintptr_t)f->privdata, offset, whence);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}

    return 0;
}

static int host_translator_op_devctl(iop_file_t *f, const char *name, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)name;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	// Intentionally not handled.

    return -48;
}

static int host_translator_op_symlink(iop_file_t *f, const char *old, const char *new_1)
{
    (void)f;

#ifndef _WIN32
	int res = symlink(old, new_1);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}
#endif

    return 0;
}

static int host_translator_op_readlink(iop_file_t *f, const char *path, char *buf, unsigned int buflen)
{
#ifndef _WIN32
	int res = readlink(path, buf, buflen);
	if (res < 0)
	{
		// TODO: translate errno
		return -errno;
	}
#endif

    return 0;
}

static int host_translator_op_ioctl2(iop_file_t *f, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen)
{
	(void)f;
	(void)cmd;
	(void)arg;
	(void)arglen;
	(void)buf;
	(void)buflen;

	// Intentionally not handled.

    return -48;
}

static iop_device_ops_t host_translator_ops = {
	&host_translator_op_init,
	&host_translator_op_exit,
	&host_translator_op_format,
	&host_translator_op_open,
	&host_translator_op_close,
	&host_translator_op_read,
	&host_translator_op_write,
	&host_translator_op_lseek,
	&host_translator_op_ioctl,
	&host_translator_op_remove,
	&host_translator_op_mkdir,
	&host_translator_op_rmdir,
	&host_translator_op_dopen,
	&host_translator_op_dclose,
	&host_translator_op_dread,
	&host_translator_op_getstat,
	&host_translator_op_chstat,
	&host_translator_op_rename,
	&host_translator_op_chdir,
	&host_translator_op_sync,
	&host_translator_op_mount,
	&host_translator_op_umount,
	&host_translator_op_lseek64,
	&host_translator_op_devctl,
	&host_translator_op_symlink,
	&host_translator_op_readlink,
	&host_translator_op_ioctl2,
};

static iop_device_t host_translator_fio_dev = {
	"host",
	(IOP_DT_FS | IOP_DT_FSEXT),
	1,
	"Host adapter filesystem",
	&host_translator_ops,
};

int host_adapter_init(void)
{
	iomanX_AddDrv(&host_translator_fio_dev);
	return 0;
}
