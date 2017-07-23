#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "iomanX_port.h"


/* copy file, host to PFS */
int copyto(const char *mount_point, const char *dest, const char *src)
{
    int retval = 0;
    int in_file = open(src, O_RDONLY |
#ifdef O_BINARY
                                O_BINARY
#else
                                0
#endif
                       );
    if (in_file != -1) {
        int result = iomanx_mount("pfs0:", mount_point, 0, NULL, 0);
        if (result >= 0) { /* mount successful */
            char dest_path[256];
            strcpy(dest_path, "pfs0:");
            strcat(dest_path, dest);

            int fh = iomanx_open(dest_path,
                                 IOMANX_O_WRONLY | IOMANX_O_CREAT, 0666);
            if (fh >= 0) {
                char buf[4096 * 16];
                int len;
                while ((len = read(in_file, buf, sizeof(buf))) > 0) {
                    result = iomanx_write(fh, buf, len);
                    if (result < 0) {
                        printf("%s: write failed with %d\n", dest_path, result);
                        retval = -1;
                        break;
                    }
                }
                if (len < 0)
                    perror(src);
                result = iomanx_close(fh);
                if (result < 0)
                    printf("close: failed with %d\n", result), retval = -1;
            } else
                printf("%s: create failed with %d\n", dest_path, fh), retval = -1;

            result = iomanx_umount("pfs0:");
            if (result < 0)
                printf("pfs0: umount failed with %d\n", result), retval = -1;
        } else
            printf("pfs0: mount of \"%s\" failed with %d\n",
                   mount_point, result),
                retval = -1;
        close(in_file);
    } else
        perror(src), retval = -1;
    return (retval);
}


/* copy file, PFS to host */
int copyfrom(const char *mount_point, const char *src, const char *dest)
{
    int retval = 0;
    int out_file = open(dest, O_CREAT | O_WRONLY |
#ifdef O_BINARY
                                  O_BINARY
#else
                                  0
#endif
                        ,
                        0664);
    if (out_file != -1) {
        int result = iomanx_mount("pfs0:", mount_point, 0, NULL, 0);
        if (result >= 0) { /* mount successful */
            char src_path[256];
            strcpy(src_path, "pfs0:");
            strcat(src_path, src);

            int fh = iomanx_open(src_path, IOMANX_O_RDONLY);
            if (fh >= 0) {
                char buf[4096 * 16];
                int len;
                while ((len = iomanx_read(fh, buf, sizeof(buf))) > 0) {
                    result = write(out_file, buf, len);
                    if (result == -1) {
                        perror(dest);
                        retval = -1;
                        break;
                    }
                }
                if (len < 0)
                    printf("%s: read failed with %d\n",
                           src_path, len),
                        retval = -1;

                result = iomanx_close(fh);
                if (result < 0)
                    printf("close: failed with %d\n", result), retval = -1;
            } else
                printf("%s: open failed with %d\n", src_path, fh), retval = -1;

            result = iomanx_umount("pfs0:");
            if (result < 0)
                printf("pfs0: umount failed with %d\n", result), retval = -1;
        } else
            printf("pfs0: mount of \"%s\" failed with %d\n",
                   mount_point, result),
                retval = -1;
        if (close(out_file) == -1)
            perror(dest), retval = -1;
    } else
        perror(dest), retval = -1;
    return (retval);
}


int list_dir_objects(int dh)
{
    int result;
    iox_dirent_t dirent;
    while ((result = iomanx_dread(dh, &dirent)) && result != -1) {
        char mode[10 + 1] = {'\0'}; /* unix-style */
        const int m = dirent.stat.mode;
        switch (m & FIO_S_IFMT) { /* item type */
            case FIO_S_IFLNK:
                mode[0] = 'l';
                break;
            case FIO_S_IFREG:
                mode[0] = '-';
                break;
            case FIO_S_IFDIR:
                mode[0] = 'd';
                break;
            default:
                mode[0] = '?';
                break;
        }
        mode[1] = m & FIO_S_IRUSR ? 'r' : '-';
        mode[2] = m & FIO_S_IWUSR ? 'w' : '-';
        mode[3] = m & FIO_S_IXUSR ? 'x' : '-';
        mode[4] = m & FIO_S_IRGRP ? 'r' : '-';
        mode[5] = m & FIO_S_IWGRP ? 'w' : '-';
        mode[6] = m & FIO_S_IXGRP ? 'x' : '-';
        mode[7] = m & FIO_S_IROTH ? 'r' : '-';
        mode[8] = m & FIO_S_IWOTH ? 'w' : '-';
        mode[9] = m & FIO_S_IXOTH ? 'x' : '-';
        mode[10] = '\0'; /* not really necessary */

        const ps2fs_datetime_t *mtime =
            (ps2fs_datetime_t *)dirent.stat.mtime;
        char mod_time[16 + 1]; /* yyyy-mm-dd hh:mi */
        sprintf(mod_time, "%04d-%02d-%02d %02d:%02d",
                mtime->year, mtime->month, mtime->day,
                mtime->hour, mtime->min);

        printf("%s %10u %s %s\n",
               mode, dirent.stat.size, mod_time, dirent.name);
    }
    return (result);
}


/* list files at PFS */
int ls(const char *mount_point, const char *path)
{
    int retval = 0;
    int result = iomanx_mount("pfs0:", mount_point, 0, NULL, 0);
    if (result >= 0) { /* mount successful */
        char dir_path[256];
        strcpy(dir_path, "pfs0:");
        strcat(dir_path, path);
        int dh = iomanx_dopen(dir_path);
        if (dh >= 0) { /* dopen successful */
#if 0
	  printf ("Directory of %s%s\n", mount_point, path);
#endif
            list_dir_objects(dh);

            result = iomanx_close(dh);
            if (result < 0)
                printf("dclose: failed with %d\n", result), retval = -1;
        } else
            printf("dopen: \"%s\" failed with %d\n",
                   dir_path, dh),
                retval = -1;

        result = iomanx_umount("pfs0:");
        if (result < 0)
            printf("pfs0: umount failed with %d\n", result), retval = -1;
    } else
        printf("pfs0: mount of \"%s\" failed with %d\n",
               mount_point, result),
            retval = -1;
    return (retval);
}


/* list HDD partitions */
int lspart(void)
{
    const char *dir_path = "hdd0:";
    int retval = 0;
    int dh = iomanx_dopen(dir_path);
    if (dh >= 0) { /* dopen successful */
#if 0
      printf ("Partitions of %s\n", dir_path);
#endif
        int result;
        iox_dirent_t dirent;
        while ((result = iomanx_dread(dh, &dirent)) && result != -1) {
            uint64_t size = (uint64_t)dirent.stat.size * 512;
            size /= (1024 * 1024);
            printf("0x%04x %5lluMB %s\n",
                   dirent.stat.mode, size, dirent.name);
        }

        result = iomanx_close(dh);
        if (result < 0)
            printf("dclose: failed with %d\n", result), retval = -1;
    } else
        printf("dopen: \"%s\" failed with %d\n",
               dir_path, dh),
            retval = dh;
    return (retval);
}


/* create PFS onto an existing partition */
int mkfs(const char *mount_point)
{
#define PFS_ZONE_SIZE 8192
#define PFS_FRAGMENT 0x00000000
    int format_arg[] = {PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT};

    return (iomanx_format("pfs:", mount_point,
                          (void *)&format_arg, sizeof(format_arg)));
}


/* create partition and (optionally) format it as PFS;
 * so far the only sizes supported are powers of 2 */
int mkpart(const char *mount_point, long size_in_mb, int format)
{
    char tmp[256];
    if (size_in_mb >= 1024)
        sprintf(tmp, "%s,%ldG", mount_point, size_in_mb / 1024);
    else
        sprintf(tmp, "%s,%ldM", mount_point, size_in_mb);
    int result = iomanx_open(tmp, IOMANX_O_RDWR | IOMANX_O_CREAT, 0);
    if (result >= 0) {
        iomanx_close(result), result = 0;

        if (format)
            result = mkfs(mount_point);
    }
    return (result);
}


/* initialize PS2 HDD with APA partitioning and create common partitions
 * (__mbr, __common, __net, etc.); common partitions are not PFS-formatted */
int initialize(void)
{
    int result = iomanx_format("hdd0:", NULL, NULL, 0);
    if (result >= 0)
        result = mkfs("hdd0:__mbr");
    return (result);
}
