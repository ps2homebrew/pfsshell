#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "iomanX_port.h"

#define IOMANX_MOUNT_POINT "pfs0:"

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options
{
    const char *device;
    const char *partition;
    const char *maxopen;
    const char *numbuffers;
    int show_help;
} options;

#define OPTION(t, p)                      \
    {                                     \
        t, offsetof(struct options, p), 1 \
    }
static const struct fuse_opt option_spec[] = {
    OPTION("--partition=%s", partition),
    OPTION("--maxopen=%s", maxopen),
    OPTION("--buffers=%s", numbuffers),
    OPTION("-h", show_help),
    OPTION("--help", show_help),
    FUSE_OPT_END};

static int iomanx_adapter_opt_proc(void *data, const char *arg, int key,
                                   struct fuse_args *outargs)
{
    (void)data;
    (void)outargs;

    switch (key) {
        case FUSE_OPT_KEY_OPT:
            return 1;
        case FUSE_OPT_KEY_NONOPT:
            if (strlen(options.device) == 0) {
                options.device = strdup(arg);
                return 0;
            }
            return 1;
        default:
            fprintf(stderr, "internal error\n");
            abort();
    }
}

static void translate_path(char *s1, const char *s2, size_t n)
{
    strncpy(s1, IOMANX_MOUNT_POINT "/", n);
    strncat(s1, s2, n);
}

static void convert_time_to_posix(time_t *posix_time, const unsigned char *iomanx_time)
{
    struct tm timeinfo;
    timeinfo.tm_sec = iomanx_time[1];
    timeinfo.tm_min = iomanx_time[2];
    timeinfo.tm_hour = iomanx_time[3];
    timeinfo.tm_mday = iomanx_time[4];
    timeinfo.tm_mon = iomanx_time[5] - 1;                               // month 1 (January) is 0
    timeinfo.tm_year = (iomanx_time[6] | (iomanx_time[7] << 8)) - 1900; // year 1900 is 0
    time_t rawtime = timegm(&timeinfo);
    // convert UTC->JST
    rawtime += (9 * 60 * 60);
    *posix_time = rawtime;
}

static void convert_time_to_iomanx(unsigned char *iomanx_time, const time_t *posix_time)
{
    struct tm timeinfo;
    time_t rawtime = *posix_time;
    // convert JST->UTC
    rawtime -= (9 * 60 * 60);
    gmtime_r(&rawtime, &timeinfo);
    iomanx_time[0] = 0;
    iomanx_time[1] = timeinfo.tm_sec;
    iomanx_time[2] = timeinfo.tm_min;
    iomanx_time[3] = timeinfo.tm_hour;
    iomanx_time[4] = timeinfo.tm_mday;
    iomanx_time[5] = timeinfo.tm_mon + 1;              // month 1 (January) is 0
    iomanx_time[6] = (timeinfo.tm_year + 1900) & 0xff; // year 1900 is 0
    iomanx_time[7] = ((timeinfo.tm_year + 1900) >> 8) & 0xff;
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
    if (FIO_S_ISLNK(*iomanx_mode)) {
        *posix_mode |= S_IFLNK;
    }
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
    if (*iomanx_mode & FIO_S_ISUID) {
        *posix_mode |= S_ISUID;
    }
    if (*iomanx_mode & FIO_S_ISGID) {
        *posix_mode |= S_ISGID;
    }
    if (*iomanx_mode & FIO_S_ISVTX) {
        *posix_mode |= S_ISVTX;
    }
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
    if (S_ISLNK(*posix_mode)) {
        *iomanx_mode |= FIO_S_IFLNK;
    }
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
    if (*posix_mode & S_ISUID) {
        *iomanx_mode |= FIO_S_ISUID;
    }
    if (*posix_mode & S_ISGID) {
        *iomanx_mode |= FIO_S_ISGID;
    }
    if (*posix_mode & S_ISVTX) {
        *iomanx_mode |= FIO_S_ISVTX;
    }
}

static void convert_stat_to_posix(struct stat *posix_stat, const iox_stat_t *iomanx_stat)
{
    memset(posix_stat, 0, sizeof(*posix_stat));
    posix_stat->st_size = iomanx_stat->size;
    if (sizeof(posix_stat->st_size) > 4) {
        posix_stat->st_size |= ((off_t)iomanx_stat->hisize << 32);
    }
    convert_mode_to_posix(&(posix_stat->st_mode), &(iomanx_stat->mode));
#if 0
    posix_stat->st_attr = iomanx_stat->attr;
#endif
    convert_time_to_posix(&(posix_stat->st_ctime), iomanx_stat->ctime);
    convert_time_to_posix(&(posix_stat->st_atime), iomanx_stat->atime);
    convert_time_to_posix(&(posix_stat->st_mtime), iomanx_stat->mtime);
#if 0
    posix_stat->st_uid = iomanx_stat->private_0;
    posix_stat->st_gid = iomanx_stat->private_1;
#endif
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
    convert_time_to_iomanx(iomanx_stat->ctime, &(posix_stat->st_ctime));
    convert_time_to_iomanx(iomanx_stat->atime, &(posix_stat->st_atime));
    convert_time_to_iomanx(iomanx_stat->mtime, &(posix_stat->st_mtime));
#if 0
    posix_stat->st_uid = iomanx_stat->private_0;
    posix_stat->st_gid = iomanx_stat->private_1;
#endif
}

static void *iomanx_adapter_init(struct fuse_conn_info *conn)
{
    (void)conn;
    return NULL;
}

static void iomanx_adapter_destroy(void *private_data)
{
    (void)private_data;
    int result = iomanx_umount(IOMANX_MOUNT_POINT);
    extern void atad_close(void); /* fake_sdk/atad.c */
    atad_close();
}

static int iomanx_adapter_open(const char *path, struct fuse_file_info *fi)
{
    if (fi == NULL) {
        return 0;
    }
    int flags = 0;
    if ((fi->flags & O_ACCMODE) == O_RDONLY) {
        flags |= IOMANX_O_RDONLY;
    }
    if ((fi->flags & O_ACCMODE) == O_WRONLY) {
        flags |= IOMANX_O_WRONLY;
    }
    if ((fi->flags & O_ACCMODE) == O_RDWR) {
        flags |= IOMANX_O_RDWR;
    }
    if (fi->flags & O_NONBLOCK) {
        flags |= IOMANX_O_NBLOCK;
    }
    if (fi->flags & O_APPEND) {
        flags |= IOMANX_O_APPEND;
    }
    if (fi->flags & O_CREAT) {
        flags |= IOMANX_O_CREAT;
    }
    if (fi->flags & O_TRUNC) {
        flags |= IOMANX_O_TRUNC;
    }
    if (fi->flags & O_EXCL) {
        flags |= IOMANX_O_EXCL;
    }
    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));
    // not handled: IOMANX_O_DIROPEN, IOMANX_O_NOWAIT
    int fh = iomanx_open(translated_path, flags);
    if (fh < 0) {
        return fh;
    }
    fi->fh = (unsigned int)fh;
    return 0;
}

static int iomanx_adapter_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    if (fi == NULL) {
        return 0;
    }
    int flags = 0;
    if ((fi->flags & O_ACCMODE) == O_RDONLY) {
        flags |= IOMANX_O_RDONLY;
    }
    if ((fi->flags & O_ACCMODE) == O_WRONLY) {
        flags |= IOMANX_O_WRONLY;
    }
    if ((fi->flags & O_ACCMODE) == O_RDWR) {
        flags |= IOMANX_O_RDWR;
    }
    if (fi->flags & O_NONBLOCK) {
        flags |= IOMANX_O_NBLOCK;
    }
    if (fi->flags & O_APPEND) {
        flags |= IOMANX_O_APPEND;
    }
    flags |= IOMANX_O_CREAT;
    if (fi->flags & O_TRUNC) {
        flags |= IOMANX_O_TRUNC;
    }
    if (fi->flags & O_EXCL) {
        flags |= IOMANX_O_EXCL;
    }
    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));
    unsigned int translated_mode = 0;
    convert_mode_to_iomanx(&translated_mode, &mode);
    // not handled: IOMANX_O_DIROPEN, IOMANX_O_NOWAIT
    int fh = iomanx_open(translated_path, flags, translated_mode);
    if (fh < 0) {
        return fh;
    }
    fi->fh = (unsigned int)fh;
    return 0;
}

static int iomanx_adapter_release(const char *path, struct fuse_file_info *fi)
{
    (void)path;
    if (fi == NULL) {
        return 0;
    }
    int res = iomanx_close(fi->fh);
    if (res < 0) {
        return res;
    }
    return 0;
}

static int iomanx_adapter_read(const char *path, char *buf, size_t size, off_t offset,
                               struct fuse_file_info *fi)
{
    size_t len;
    int res;
    (void)fi;
    if (fi == NULL) {
        return -ENOENT;
    }
    res = iomanx_lseek64(fi->fh, offset, IOMANX_SEEK_SET);
    if (res == -48) {
        res = iomanx_lseek(fi->fh, offset, IOMANX_SEEK_SET);
    }
    if (res < 0) {
        return res;
    }
    res = iomanx_read(fi->fh, buf, size);
    return res;
}

static int iomanx_adapter_write(const char *path, const char *buf, size_t size,
                                off_t offset, struct fuse_file_info *fi)
{
    int fh;
    int res;

    if (fi == NULL) {
        return -ENOENT;
    }
    res = iomanx_lseek64(fi->fh, offset, IOMANX_SEEK_SET);
    if (res == -48) {
        res = iomanx_lseek(fi->fh, offset, IOMANX_SEEK_SET);
    }
    if (res < 0) {
        return res;
    }
    res = iomanx_write(fi->fh, (void *)buf, size);
    return res;
}

static int iomanx_adapter_unlink(const char *path)
{
    int res;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_remove(translated_path);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_mkdir(const char *path, mode_t mode)
{
    int res;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    unsigned int translated_mode = 0;
    convert_mode_to_iomanx(&translated_mode, &mode);

    res = iomanx_mkdir(translated_path, translated_mode);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_rmdir(const char *path)
{
    int res;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_rmdir(translated_path);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                                  off_t offset, struct fuse_file_info *fi)
{
    int dp;
    int result = 0;
    iox_dirent_t de;

    (void)offset;
    (void)fi;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    dp = iomanx_dopen(translated_path);
    if (dp < 0) {
        return dp;
    }

    while ((result = iomanx_dread(dp, &de)) && (result != -1)) {
        struct stat st;
        convert_stat_to_posix(&st, &(de.stat));
        if (filler(buf, de.name, &st, 0))
            break;
    }

    iomanx_close(dp);
    return result;
}


static int iomanx_adapter_getattr(const char *path, struct stat *stbuf)
{
    int res;
    iox_stat_t stat;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_getstat(translated_path, &stat);
    if (res < 0) {
        return res;
    }
    convert_stat_to_posix(stbuf, &stat);

    return 0;
}

static int iomanx_adapter_chmod(const char *path, mode_t mode)
{
    int res;
    iox_stat_t iomanx_stat;
    struct stat posix_stat;
    posix_stat.st_mode = mode;
    convert_stat_to_iomanx(&iomanx_stat, &posix_stat);

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_chstat(translated_path, &stat, FIO_CST_MODE);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_utimens(const char *path, const struct timespec ts[2])
{
    int res;
    iox_stat_t stat;
    convert_time_to_iomanx(stat.atime, &(ts[0].tv_sec));
    convert_time_to_iomanx(stat.mtime, &(ts[1].tv_sec));

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_chstat(translated_path, &stat, FIO_CST_AT | FIO_CST_MT);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_rename(const char *from, const char *to)
{
    int res;

    char translated_from[1024];
    translate_path(translated_from, from, sizeof(translated_from));

    char translated_to[1024];
    translate_path(translated_to, to, sizeof(translated_to));

    res = iomanx_rename(translated_from, translated_to);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_symlink(const char *from, const char *to)
{
    int res;

    char translated_from[1024];
    translate_path(translated_from, from, sizeof(translated_from));

    char translated_to[1024];
    translate_path(translated_to, to, sizeof(translated_to));

    res = iomanx_symlink(translated_from, translated_to);
    if (res < 0) {
        return res;
    }

    return 0;
}

static int iomanx_adapter_readlink(const char *path, char *buf, size_t size)
{
    int res;

    char translated_path[1024];
    translate_path(translated_path, path, sizeof(translated_path));

    res = iomanx_readlink(translated_path, buf, size - 1);
    if (res < 0) {
        return res;
    }

    buf[res] = '\0';
    return 0;
}

static const struct fuse_operations iomanx_adapter_operations = {
    .init = iomanx_adapter_init,
    .destroy = iomanx_adapter_destroy,
    .open = iomanx_adapter_open,
    .create = iomanx_adapter_create,
    .release = iomanx_adapter_release,
    .read = iomanx_adapter_read,
    .write = iomanx_adapter_write,
    .unlink = iomanx_adapter_unlink,
    .mkdir = iomanx_adapter_mkdir,
    .rmdir = iomanx_adapter_rmdir,
    .readdir = iomanx_adapter_readdir,
    .getattr = iomanx_adapter_getattr,
    .chmod = iomanx_adapter_chmod,
    .utimens = iomanx_adapter_utimens,
    .rename = iomanx_adapter_rename,
    .symlink = iomanx_adapter_symlink,
    .readlink = iomanx_adapter_readlink,
};

static void show_help(const char *progname)
{
    printf("usage: %s [options] <disk> <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --partition=<s>      PFS partition in APA to mount\n"
           "                         (default \"__common\")\n"
           "    --maxopen=<d>        Maximum number of files that can be opened at one time\n"
           "                         (default \"32\", max \"32\")\n"
           "    --numbuffers=<d>     Maximum number of buffers\n"
           "                         (default \"127\", max \"127\")\n"
           "\n");
}

int main(int argc, char *argv[])
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    /* Set defaults -- we have to use strdup so that
       fuse_opt_parse can free the defaults if other
       values are specified */
    options.device = strdup("");
    options.partition = strdup("__common");
    options.maxopen = strdup("32");
    options.numbuffers = strdup("127");

    /* Parse options */
    if (fuse_opt_parse(&args, &options, option_spec, iomanx_adapter_opt_proc) == -1)
        return 1;

    if (strlen(options.device) == 0 || options.show_help) {
        show_help(argv[0]);
        if (fuse_opt_add_arg(&args, "--help") != 0) {
            return 1;
        }
        args.argv[0][0] = '\0';
    }

    /* where (image of) PS2 HDD is; in fake_sdk/atad.c */
    extern char atad_device_path[256];
    strncpy(atad_device_path, options.device, sizeof(atad_device_path));

    /* mandatory */
    int result = _init_apa(0, NULL);
    if (result < 0) {
        fprintf(stderr, "(!) init_apa: failed with %d (%s)\n", result,
                strerror(-result));
        return 1;
    }

    static const char *pfs_args[] =
        {
            "pfs.irx",
            "-m", "1",
            "-o", NULL,
            "-n", NULL,
            NULL};

    pfs_args[4] = options.maxopen;
    pfs_args[6] = options.numbuffers;

    /* mandatory */
    result = _init_pfs(7, (char **)pfs_args);
    if (result < 0) {
        fprintf(stderr, "(!) init_pfs: failed with %d (%s)\n", result,
                strerror(-result));
        return 1;
    }

    char mount_point[256];
    strncpy(mount_point, "hdd0:", sizeof(mount_point));
    strncat(mount_point, options.partition, sizeof(mount_point) - strlen(mount_point) - 1);
    result = iomanx_mount(IOMANX_MOUNT_POINT, mount_point, 0, NULL, 0);
    if (result < 0) {
        fprintf(stderr, "(!) %s: %s.\n", mount_point, strerror(-result));
        return 1;
    }

    ret = fuse_main(args.argc, args.argv, &iomanx_adapter_operations, NULL);
    fuse_opt_free_args(&args);
    return ret;
}
