#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

#include "iomanX_port.h"

#define IOMANX_PATH_MAX    256
#define IOMANX_MOUNT_POINT "pfs0:"

/* Mostly based on musl libc's nftw implementation 35e9831156efc1b54e1a91917ba0f787d5df3353 */

typedef struct path_info
{
    int orig_len;
    const char *path_prefix;
} path_info_t;

typedef int (*wrapped_ftw_callback)(path_info_t *pi, const char *path, const iox_stat_t *st);

static int do_wrapped_ftw(path_info_t *pi, char *path, wrapped_ftw_callback fn)
{
    size_t l = strlen(path), j = l && path[l - 1] == '/' ? l - 1 : l;
    iox_stat_t st;
    int r;

    if (iomanX_getstat(path, &st) < 0) {
        return -1;
    }

    if ((r = fn(pi, path, &st))) {
        return r;
    }

    if (FIO_S_ISDIR(st.mode)) {
        int d = iomanX_dopen(path);
        if (d >= 0) {
            int result;
            iox_dirent_t de;

            while ((result = iomanX_dread(d, &de)) && result != -1) {
                if (de.name[0] == '.' && (!de.name[1] || (de.name[1] == '.' && !de.name[2])))
                    continue;
                if (strlen(de.name) >= IOMANX_PATH_MAX - l) {
                    iomanX_close(d);
                    return -1;
                }
                path[j] = '/';
                strcpy(path + j + 1, de.name);
                if ((r = do_wrapped_ftw(pi, path, fn))) {
                    iomanX_close(d);
                    return r;
                }
            }
            iomanX_close(d);
        } else {
            return -1;
        }
    }

    path[l] = 0;

    return 0;
}

static int wrapped_ftw(const char *path_prefix, const char *path, wrapped_ftw_callback fn)
{
    int r, cs;
    size_t l;
    path_info_t pi;
    char pathbuf[IOMANX_PATH_MAX + 1];

    l = strlen(path);
    if (l > IOMANX_PATH_MAX) {
        return -1;
    }
    memcpy(pathbuf, path, l + 1);

    pi.orig_len = l;
    pi.path_prefix = path_prefix;

    r = do_wrapped_ftw(&pi, pathbuf, fn);
    return r;
}

/* Mostly based on sltar code */

static time_t convert_iox_stat_time_to_posix(const unsigned char *iomanx_time)
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
    return rawtime;
}

static unsigned int convert_mode_to_posix(unsigned int iomanx_mode)
{
    unsigned int posix_mode = 0;
    if (FIO_S_ISDIR(iomanx_mode)) {
        posix_mode |= /* S_IFDIR */ 0040000;
    }
    if (FIO_S_ISREG(iomanx_mode)) {
        posix_mode |= /* S_IFREG */ 0100000;
    }
    if (FIO_S_ISLNK(iomanx_mode)) {
        posix_mode |= /* S_IFLNK */ 0120000;
    }
    if (iomanx_mode & FIO_S_IRUSR) {
        posix_mode |= /* S_IRUSR */ 0400;
    }
    if (iomanx_mode & FIO_S_IWUSR) {
        posix_mode |= /* S_IWUSR */ 0200;
    }
    if (iomanx_mode & FIO_S_IXUSR) {
        posix_mode |= /* S_IXUSR */ 0100;
    }
    if (iomanx_mode & FIO_S_IRGRP) {
        posix_mode |= /* S_IRGRP */ 0040;
    }
    if (iomanx_mode & FIO_S_IWGRP) {
        posix_mode |= /* S_IWGRP */ 0020;
    }
    if (iomanx_mode & FIO_S_IXGRP) {
        posix_mode |= /* S_IXGRP */ 0010;
    }
    if (iomanx_mode & FIO_S_IROTH) {
        posix_mode |= /* S_IROTH */ 0004;
    }
    if (iomanx_mode & FIO_S_IWOTH) {
        posix_mode |= /* S_IWOTH */ 0002;
    }
    if (iomanx_mode & FIO_S_IXOTH) {
        posix_mode |= /* S_IXOTH */ 0001;
    }
    if (iomanx_mode & FIO_S_ISUID) {
        posix_mode |= /* S_ISUID */ 04000;
    }
    if (iomanx_mode & FIO_S_ISGID) {
        posix_mode |= /* S_ISGID */ 02000;
    }
    if (iomanx_mode & FIO_S_ISVTX) {
        posix_mode |= /* S_ISVTX */ 01000;
    }
    return posix_mode;
}

enum TarHeader {
    NAME = 0,
    MODE = 100,
    SIZE = 124,
    MTIME = 136,
    CHK = 148,
    TYPE = 156,
    LINK = 157,
    MAGIC = 257,
    VERS = 263,
    NAME2 = 345,
    END = 512
};

// Converts a POSIX time_t value to an 8-byte iomanX timestamp format
static void convert_posix_time_to_iox_time(time_t posix_time, unsigned char *iomanx_time)
{
    struct tm *tm_time;

    // Convert UTC to JST (revert +9h used in the forward function)
    posix_time -= 9 * 60 * 60;

    // Convert time_t to tm struct in UTC
    tm_time = gmtime(&posix_time);

    // Encode into iomanX format
    iomanx_time[0] = 0; // unused
    iomanx_time[1] = tm_time->tm_sec;
    iomanx_time[2] = tm_time->tm_min;
    iomanx_time[3] = tm_time->tm_hour;
    iomanx_time[4] = tm_time->tm_mday;
    iomanx_time[5] = tm_time->tm_mon + 1; // tm_mon is 0-based
    iomanx_time[6] = tm_time->tm_year + 1900;
    iomanx_time[7] = (tm_time->tm_year + 1900) >> 8;
}

static unsigned int convert_posix_mode_to_iomanx(unsigned int posix_mode)
{
    unsigned int iomanx_mode = 0;

    if ((posix_mode & 0170000) == 0040000)
        iomanx_mode |= FIO_S_IFDIR;
    if ((posix_mode & 0170000) == 0100000)
        iomanx_mode |= FIO_S_IFREG;
    if ((posix_mode & 0170000) == 0120000)
        iomanx_mode |= FIO_S_IFLNK;

    if (posix_mode & 0400)
        iomanx_mode |= FIO_S_IRUSR;
    if (posix_mode & 0200)
        iomanx_mode |= FIO_S_IWUSR;
    if (posix_mode & 0100)
        iomanx_mode |= FIO_S_IXUSR;

    if (posix_mode & 0040)
        iomanx_mode |= FIO_S_IRGRP;
    if (posix_mode & 0020)
        iomanx_mode |= FIO_S_IWGRP;
    if (posix_mode & 0010)
        iomanx_mode |= FIO_S_IXGRP;

    if (posix_mode & 0004)
        iomanx_mode |= FIO_S_IROTH;
    if (posix_mode & 0002)
        iomanx_mode |= FIO_S_IWOTH;
    if (posix_mode & 0001)
        iomanx_mode |= FIO_S_IXOTH;

    if (posix_mode & 04000)
        iomanx_mode |= FIO_S_ISUID;
    if (posix_mode & 02000)
        iomanx_mode |= FIO_S_ISGID;
    if (posix_mode & 01000)
        iomanx_mode |= FIO_S_ISVTX;

    return iomanx_mode;
}


static void tar_checksum(const char b[END], char *chk)
{
    unsigned sum = 0, i;
    for (i = 0; i < END; i++)
        sum += (i >= CHK && i < CHK + 8) ? ' ' : b[i];
    snprintf(chk, 8, "%.7o", sum);
}

static FILE *tarfile_handle = NULL;

static int tar_c_file(path_info_t *pi, const char *in_path, const iox_stat_t *st)
{
    int l = END;
    char b[END] = {0};
    int f = -1;
    int pathlen;
    char path[512];

    /* TODO: pax header for longer filenames and larger files */
    /* https://pubs.opengroup.org/onlinepubs/9699919799/utilities/pax.html#tag_20_92_13 */

    if ((FIO_S_ISREG(st->mode)) && (st->hisize != 0)) {
        /* The file is over 4GB, which we don't support (currently) */
        return 0;
    }

    snprintf(path, sizeof(path), "%s%s", pi->path_prefix, in_path + pi->orig_len);

    pathlen = strlen(path);

    if (pathlen == 0) {
        /* We don't need to archive the root */
        return 0;
    }

    memset(b + SIZE, '0', 11);
    memcpy(b + MAGIC, "ustar\x00", 6);
    memcpy(b + VERS, "00", 2);
    if (pathlen > 100) {
        char *path_separate = strchr(path, '/');

        if (path_separate == NULL) {
            /* Path is too long */
            return 0;
        }

        while ((path_separate - path) < sizeof(path)) {
            if (*path_separate == '\x00') {
                break;
            }
            char *new_path_separate = strchr(path_separate + 1, '/');
            if (new_path_separate == NULL) {
                break;
            }
            if ((new_path_separate - path) >= 155) {
                break;
            }
            path_separate = new_path_separate;
        }

        if ((path_separate - path) >= 155) {
            /* Path is too long */
            return 0;
        }

        {
            int prefix_pathlen = path_separate - path;

            memcpy(b + NAME2, path, prefix_pathlen);
            memcpy(b + NAME, path + prefix_pathlen, pathlen - prefix_pathlen);
        }

    } else {
        memcpy(b + NAME, path, pathlen);
    }
    snprintf(b + MODE, 8, "%.7o", (unsigned int)(convert_mode_to_posix(st->mode)));
    snprintf(b + MTIME, 12, "%.11o", (unsigned int)(convert_iox_stat_time_to_posix(st->mtime)));

    if (FIO_S_ISREG(st->mode)) {
        b[TYPE] = '0';
        snprintf(b + SIZE, 12, "%.11o", (unsigned int)(st->size));
        f = iomanX_open(in_path, FIO_O_RDONLY);
    } else if (FIO_S_ISDIR(st->mode)) {
        b[TYPE] = '5';
    } else if (FIO_S_ISLNK(st->mode)) {
        b[TYPE] = '2';
        iomanX_readlink(in_path, b + LINK, 100);
    }
    tar_checksum(b, b + CHK);
    do {
        if (l < END) {
            memset(b + l, 0, END - l);
        }
        fwrite(b, END, 1, tarfile_handle);
    } while ((f >= 0) && ((l = iomanX_read(f, b, END)) > 0));
    iomanX_close(f);
    return 0;
}

static int tar_part(const char *arg)
{
    int retval = 0;
    int dh = iomanX_dopen("hdd0:");
    if (dh >= 0) {
        int result;
        iox_dirent_t de;
        while ((result = iomanX_dread(dh, &de)) && result != -1) {
            if (de.stat.mode == 0x0100 && de.stat.attr != 1) {
                printf("(%s) %s\n", "hdd0:", de.name);
                if (arg == NULL || !strcmp(de.name, arg)) {
                    char mount_point[256];
                    char prefix_path[256];

                    snprintf(mount_point, sizeof(mount_point), "hdd0:%s", de.name);

                    result = iomanX_mount(IOMANX_MOUNT_POINT, mount_point, FIO_MT_RDONLY, NULL, 0);
                    if (result < 0) {
                        fprintf(stderr, "(!) %s: %s.\n", mount_point, strerror(-result));
                        continue;
                    }

                    snprintf(prefix_path, sizeof(prefix_path), "%s/", de.name);
                    wrapped_ftw(prefix_path, IOMANX_MOUNT_POINT "/", tar_c_file);

                    iomanX_umount(IOMANX_MOUNT_POINT);
                }
            }
        }

        result = iomanX_close(dh);
        if (result < 0) {
            printf("(!) dclose: failed with %d\n", result);
            retval = -1;
        }
    } else {
        printf("(!) dopen: failed with %d\n", dh);
        retval = dh;
    }
    return retval;
}

static void ensure_parent_dirs_exist(const char *path)
{
    char tmp[IOMANX_PATH_MAX];
    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';
    int result;

    for (char *p = tmp + strlen(IOMANX_MOUNT_POINT) + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            result = iomanX_mkdir(tmp, 0777); // Ignore errors
            *p = '/';
        }
    }
}

static int restore_from_tar(const char *pfs_mount_path, const char *prefix_path)
{
    char header[512];

    size_t prefix_len = strlen(prefix_path);

    while (fread(header, 1, 512, tarfile_handle) == 512) {
        if (header[0] == '\0')
            break;

        // Name
        char name[256] = {0};
        strncpy(name, header + NAME, 100);
        if (name[0] == '\0')
            continue;

        if (strncmp(name, prefix_path, prefix_len) != 0)
            continue;

        const char *rel_path = name + prefix_len;

        if (*rel_path == '\0')
            continue;

        // Type and size
        char size_str[13] = {0};
        strncpy(size_str, header + SIZE, 12);
        unsigned int size = strtol(size_str, NULL, 8);
        char type = header[TYPE];

        // Full path = pfs0:/ + relative path
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s%s", pfs_mount_path, rel_path);

        // Create directories
        ensure_parent_dirs_exist(full_path);

        // Parse mtime from tar header
        char mtime_str[13] = {0};
        strncpy(mtime_str, header + MTIME, 12);
        time_t mtime = strtol(mtime_str, NULL, 8);
        // Extract mode from tar header (octal format)
        char mode_str[8] = {0};
        strncpy(mode_str, header + MODE, 7);
        unsigned int posix_mode = strtol(mode_str, NULL, 8);
        unsigned int iomanx_mode = convert_posix_mode_to_iomanx(posix_mode);

        if (type == '5') {
            iomanX_mkdir(full_path, 0777);

            // Convert to iomanX format
            iox_stat_t st_time = {0};
            convert_posix_time_to_iox_time(mtime, st_time.mtime);
            memcpy(st_time.atime, st_time.mtime, sizeof(st_time.mtime));
            memcpy(st_time.ctime, st_time.mtime, sizeof(st_time.mtime));
            iox_stat_t st = {0};
            st.mode = iomanx_mode;
            iomanX_chstat(full_path, &st, FIO_CST_MODE);
            // Set timestamps
            iomanX_chstat(full_path, &st_time, FIO_CST_MT | FIO_CST_AT | FIO_CST_CT);
        } else if (type == '0' || type == '\0') {
            int fd = iomanX_open(full_path, FIO_O_WRONLY | FIO_O_CREAT | FIO_O_TRUNC, 0666);

            if (fd < 0) {
                fprintf(stderr, "(!) Failed to create file: %s\n", full_path);
                fseek(tarfile_handle, ((size + 511) / 512) * 512, SEEK_CUR);
                continue;
            }

            // Read and write file content
            unsigned int remaining = size;
            while (remaining > 0) {
                char buf[512];
                int chunk = remaining > 512 ? 512 : remaining;
                fread(buf, 1, chunk, tarfile_handle);
                iomanX_write(fd, buf, chunk);
                remaining -= chunk;
            }
            iomanX_close(fd);

            // Convert to iomanX format
            iox_stat_t st_time = {0};
            convert_posix_time_to_iox_time(mtime, st_time.mtime);
            memcpy(st_time.atime, st_time.mtime, sizeof(st_time.mtime));
            memcpy(st_time.ctime, st_time.mtime, sizeof(st_time.mtime));
            iox_stat_t st = {0};
            st.mode = iomanx_mode;
            iomanX_chstat(full_path, &st, FIO_CST_MODE);
            // Set timestamps
            iomanX_chstat(full_path, &st_time, FIO_CST_MT | FIO_CST_AT | FIO_CST_CT);
            printf("Restoring: %s (type: %c, size: %u, mask: %o, time: %lu)\n", name, type, size, posix_mode, mtime);

            // Skip padding
            if (size % 512 != 0)
                fseek(tarfile_handle, 512 - (size % 512), SEEK_CUR);
        } else {
            // Unknown type
            fseek(tarfile_handle, ((size + 511) / 512) * 512, SEEK_CUR);
        }
    }

    return 0;
}

static int part_tar(const char *arg)
{
    int retval = 0;
    int dh = iomanX_dopen("hdd0:");
    if (dh >= 0) {
        int result;
        iox_dirent_t de;
        while ((result = iomanX_dread(dh, &de)) && result != -1) {
            if (de.stat.mode == 0x0100 && de.stat.attr != 1) {
                printf("(%s) %s\n", "hdd0:", de.name);
                if (arg == NULL || !strcmp(de.name, arg)) {
                    char mount_point[256];
                    char prefix_path[256];

                    snprintf(mount_point, sizeof(mount_point), "hdd0:%s", de.name);

                    result = iomanX_mount(IOMANX_MOUNT_POINT, mount_point, FIO_MT_RDWR, NULL, 0);
                    if (result < 0) {
                        fprintf(stderr, "(!) %s: %s.\n", mount_point, strerror(-result));
                        continue;
                    }

                    snprintf(prefix_path, sizeof(prefix_path), "%s/", de.name);
                    restore_from_tar(IOMANX_MOUNT_POINT "/", prefix_path);

                    iomanX_umount(IOMANX_MOUNT_POINT);
                }
            }
        }

        result = iomanX_close(dh);
        if (result < 0) {
            printf("(!) dclose: failed with %d\n", result);
            retval = -1;
        }
    } else {
        printf("(!) dopen: failed with %d\n", dh);
        retval = dh;
    }
    return retval;
}

static void show_help(const char *progname)
{
    printf("usage: %s --extract <ps2_hdd_device_path> [--partition <optional_partition_name>] [<tar_file>]\n", progname);
    printf("usage: %s --restore <ps2_hdd_device_path> --partition <mandatory_partition_name> <tar_file>\n", progname);
}

/* where (image of) PS2 HDD is; in fake_sdk/atad.c */
extern void set_atad_device_path(const char *path);

extern void atad_close(void); /* fake_sdk/atad.c */

int main(int argc, char *argv[])
{
    int result;
    bool extract_mode = false;

    if (argc < 4) {
        show_help(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--extract") == 0)
        extract_mode = true;
    else if (strcmp(argv[1], "--restore") == 0)
        extract_mode = false;
    else {
        show_help(argv[0]);
        return 1;
    }
    if (argc < 6 && !extract_mode) {
        fprintf(stderr, "(!) Missing mandatory arguments for restore mode.\n");
        show_help(argv[0]);
        return 1;
    }

    // Get the input HDD image path
    const char *hdd_path = argv[2];
    const char *last_slash = strrchr(hdd_path, '/');
    const char *filename = last_slash ? last_slash + 1 : hdd_path;

    // Find the last dot for extension
    const char *last_dot = strrchr(filename, '.');
    size_t base_len = last_dot ? (size_t)(last_dot - filename) : strlen(filename);
    char tar_filename[1024];
    const char *partition_name = NULL;
    if (argc >= 4) {
        if (strncmp(argv[3], "--partition", 11) == 0) {
            if (argc < 5) {
                fprintf(stderr, "(!) Missing partition name.\n");
                show_help(argv[0]);
                return 1;
            }
            partition_name = argv[4];
            if (argc > 5)
                snprintf(tar_filename, sizeof(tar_filename), "%s", argv[5]);
            else
                snprintf(tar_filename, sizeof(tar_filename), "%s_%.*s.tar", partition_name, (int)base_len, filename);
        } else if (!extract_mode) {
            show_help(argv[0]);
            return 1;
        } else {
            snprintf(tar_filename, sizeof(tar_filename), "%s", argv[3]);
        }
    } else
        snprintf(tar_filename, sizeof(tar_filename), "%.*s.tar", (int)base_len, filename);

    // Check if tar file already exists for extract mode
    if (extract_mode) {
        FILE *test_file = fopen(tar_filename, "rb");
        if (test_file != NULL) {
            fclose(test_file);
            fprintf(stderr, "(!) %s already exists. Aborting.\n", tar_filename);
            return 1;
        }
    }

    set_atad_device_path(argv[2]);
    static const char *apa_args[] =
        {
            "ps2hdd.irx",
            "-o", "2",
            NULL};

    /* mandatory */
    result = _init_apa(3, (char **)apa_args);
    if (result < 0) {
        fprintf(stderr, "(!) init_apa: failed with %d (%s)\n", result,
                strerror(-result));
        return 1;
    }

    static const char *pfs_args[] =
        {
            "pfs.irx",
            "-m", "1",
            "-o", "32",
            "-n", "127",
            NULL};

    /* mandatory */
    result = _init_pfs(7, (char **)pfs_args);
    if (result < 0) {
        fprintf(stderr, "(!) init_pfs: failed with %d (%s)\n", result,
                strerror(-result));
        return 1;
    }

    if (extract_mode)
        tarfile_handle = fopen(tar_filename, "wb");
    else
        tarfile_handle = fopen(tar_filename, "rb");
    if (tarfile_handle == NULL) {
        fprintf(stderr, "(!) %s: %s.\n", tar_filename, strerror(errno));
        return 1;
    }
    if (extract_mode) {
        printf("Extracting from %s to %s\n", hdd_path, tar_filename);
        tar_part(partition_name);
    } else {
        printf("Restoring from %s to %s\n", tar_filename, hdd_path);
        part_tar(partition_name);
    }

    fclose(tarfile_handle);

    // Check if the tar file is empty (no files dumped)
    FILE *check_file = fopen(tar_filename, "rb");
    if (check_file != NULL) {
        fseek(check_file, 0, SEEK_END);
        long size = ftell(check_file);
        fclose(check_file);
        if (size == 0) {
            remove(tar_filename);
            printf("Tar file empty, tar file removed: %s\n", tar_filename);
        }
    }

    atad_close();

    return 0;
}
