
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include "pfslib_compat.h"
#include <errno.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <unistd.h>

static int hddFd[2];
static int g_hdd_argc = 5;
static int g_pfs_argc = 7;
static char *g_hdd_argv[] = {
    NULL,
    "-o",
    "32",
    "-n",
    "8",
    NULL,
};
static char *g_pfs_argv[] = {
    NULL,
    "-m",
    "32",
    "-n",
    "72",
    "-o",
    "32",
    NULL,
};

#define hdd_main _init_apa
#define pfs_main _init_pfs

/* where (image of) PS2 HDD is; in fake_sdk/atad.c */
extern int set_atad_device_handle(int fd);

int scepfsInit(void)
{
    int blkdev_count;
    int i;
    char dev_buf[16];

    blkdev_count = 0;
    for (i = 0; i < (sizeof(hddFd) / sizeof(hddFd[0])); i += 1) {
        sprintf(dev_buf, "/dev/hd%c", 'a' + i);
        hddFd[i] = open(dev_buf, O_LARGEFILE | O_RDWR);
        if (hddFd[i] < 0) {
            hddFd[i] = -1;
            continue;
        }
        if (flock(hddFd[i], LOCK_EX)) {
            close(hddFd[i]);
            hddFd[i] = -1;
            continue;
        }
#ifndef WIP_WIP_FIXME
        if (i == 0) {
            if (set_atad_device_handle(hddFd[i])) {
                flock(hddFd[i], LOCK_UN);
                close(hddFd[i]);
                continue;
            }
        }
#endif
        blkdev_count += 1;
    }
    return !blkdev_count ? -1 : (hdd_main(g_hdd_argc, g_hdd_argv) ? -2 : (pfs_main(g_pfs_argc, g_pfs_argv) ? -3 : 0));
}

void scepfsExit(void)
{
    int i;

#ifdef WIP_WIP_FIXME
    Flush1024Buffer();
#endif
    for (i = 0; i < (sizeof(hddFd) / sizeof(hddFd[0])); i += 1) {
        if (hddFd[i] >= 0) {
            flock(hddFd[i], LOCK_UN);
            close(hddFd[i]);
        }
    }
}

#ifdef WIP_WIP_FIXME
static char Buf1024[0x200];
static int Buf1024dev;
static int Buf1024lba;
static int fDirty1024;

int rawDiskAccess(int device, void *buf, unsigned int lba, int nsectors, int dir)
{
    return ((lseek64(hddFd[device], lba << 9, SEEK_SET) < 0) || ((dir ? write(hddFd[device], buf, nsectors << 9) : read(hddFd[device], buf, nsectors << 9)) != nsectors << 9)) ? -1 : 0;
}

void Flush1024Buffer(void)
{
    if (fDirty1024) {
        fDirty1024 = 0;
        rawDiskAccess(Buf1024dev, &Buf1024[0], Buf1024lba, 1, 1);
    }
}

int DiskAccess(int device, char *buf, unsigned int lba, int nsectors, int dir)
{
    char *buf_tmp;
    int lba_tmp;
    int nsectors_tmp;

    buf_tmp = buf;
    lba_tmp = lba;
    nsectors_tmp = nsectors;
    if (fDirty1024 == 1) {
        if (dir && device == Buf1024dev && Buf1024lba + 1 == lba) {
            char buf_stk[1024];

            lba_tmp = Buf1024lba + 2;
            nsectors_tmp = nsectors - 1;
            memcpy(&buf_stk[0], &Buf1024[0], sizeof(Buf1024));
            memcpy(&buf_stk[512], buf, 512);
            rawDiskAccess(device, &buf_stk[0], Buf1024lba, 2, 1);
            fDirty1024 = 0;
            buf_tmp = &buf[512];
            if (!nsectors_tmp)
                return 0;
        } else
            Flush1024Buffer();
    }
    if (dir != 1 || ((lba_tmp + nsectors_tmp - 1) & 1))
        return rawDiskAccess(device, buf_tmp, lba_tmp, nsectors_tmp, dir);
    Buf1024lba = lba_tmp + nsectors_tmp - 1;
    Buf1024dev = device;
    memcpy(&Buf1024[0], &buf_tmp[512 * nsectors_tmp], 512);
    nsectors_tmp -= 1;
    fDirty1024 = 1;
    return nsectors_tmp ? rawDiskAccess(device, buf_tmp, lba_tmp, nsectors_tmp, dir) : 0;
}

int DiskGetNumberOfSectors(int device)
{
    int ret_sz;

    if (hddFd[device] < 0)
        return 0;
    ioctl(hddFd[device], BLKGETSIZE, &ret_sz);
    return ret_sz;
}

void DiskSync(int device)
{
    Flush1024Buffer();
    fsync(hddFd[device]);
}
#endif

int scepfsSetReadAhead(int enabled)
{
    int raget_res;
    int ret_ra;

    scepfsGetReadAhead();
    raget_res = ioctl(hddFd[0], BLKRASET, enabled);
    if (raget_res) {
        printf("BLKRASET failed. errno%d\n", errno);
        return raget_res;
    }
    ioctl(hddFd[0], BLKRAGET, &ret_ra);
    printf("read ahead setting set to %d\n", ret_ra);
    return 0;
}

int scepfsGetReadAhead(void)
{
    int ret_ra;

    ioctl(hddFd[0], BLKRAGET, &ret_ra);
    printf("current read ahead setting = %d\n", ret_ra);
    return ret_ra;
}
