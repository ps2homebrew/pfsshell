
#define _FILE_OFFSET_BITS 64

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winioctl.h>
#include <io.h>
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include <sys/disk.h>
#endif

typedef uint32_t u32;
typedef uint64_t u64;

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* These are used with the dir parameter of ata_device_sector_io().  */
#define ATA_DIR_READ  0
#define ATA_DIR_WRITE 1

typedef struct _ata_devinfo
{
    u32 exists;          /* Was successfully probed.  */
    u32 has_packet;      /* Supports the PACKET command set.  */
    u32 total_sectors;   /* Total number of user sectors.  */
    u32 security_status; /* Word 0x100 of the identify info.  */
} ata_devinfo_t;

static int handle = -1;

static u32 hdd_length = 0; /* in sectors */

int set_atad_device_handle(int fd)
{
#ifdef _WIN32
    HANDLE win_handle = (HANDLE)_get_osfhandle(fd);
#endif
#ifdef __APPLE__
    u64 size = 0, sector_count = 0;
    u32 sector_size = 0;
    if (ioctl(fd, DKIOCGETBLOCKCOUNT, &sector_count) == 0) {
        ioctl(fd, DKIOCGETBLOCKSIZE, &sector_size);
        if (sector_size != 512)
            size = ((sector_count * sector_size) - 511) / 512;
        else
            size = sector_count;
        if ((int64_t)size >= 0)
            hdd_length = size;
        else
            return 1;
    } else if (errno == ENOTTY) {
        /* Not a device. Fall back to lseek */
#endif
#ifdef _WIN32
        DISK_GEOMETRY geo;
        DWORD len = 0;
        u64 size = 0, size_in_bytes = 0;
        if (win_handle != INVALID_HANDLE_VALUE && DeviceIoControl(win_handle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &geo, sizeof(DISK_GEOMETRY), &len, NULL)) {
            size_in_bytes = (geo.Cylinders.QuadPart * geo.TracksPerCylinder * geo.SectorsPerTrack * geo.BytesPerSector);
            size = ((size_in_bytes)-511) / 512;
            if ((int64_t)size >= 0) {
                hdd_length = size;
            } else
                return 1;
        } else {
#endif
            off_t size = lseek(fd, 0, SEEK_END);
            if (size != (off_t)-1)
                hdd_length = (size - 511) / 512;
            else
                return 1;
#ifdef _WIN32
        }
#endif
#ifdef __APPLE__
    }
#endif
    handle = fd;
    return 0;
}

void set_atad_device_path(const char *path)
{
    int fd;
    fd = open(path, O_RDWR | O_BINARY);
    if (fd == -1 || set_atad_device_handle(fd)) {
        perror(path), exit(1);
    }
}

void atad_close(void)
{
    if (handle != -1)
        close(handle), handle = -1;
}

ata_devinfo_t *ata_get_devinfo(int device)
{
    static ata_devinfo_t info;
    if (device == 0 && handle != -1) {
        info.exists = 1;
        info.has_packet = 0;
        info.total_sectors = hdd_length;
        info.security_status = 0;
    } else {
        info.exists = 0;
        info.has_packet = 0;
        info.total_sectors = 0;
        info.security_status = 0;
    }
    return (&info);
}

int ata_device_sector_io(int device, void *buf, u32 lba, u32 nsectors, int dir)
{
    if (handle == -1) {
        printf("atadDmaTransfer: device fd not set\n");
        return (-1);
    }

    if (device != 0) {
        printf("atadDmaTransfer: invalid device %d\n", device);
        return (-1);
    }

    off_t pos = lseek(handle, (off_t)lba * 512, SEEK_SET);
    if (pos == (off_t)-1) {
        printf("lseek: atad device fd %d: %s\n", handle, strerror(errno));
        return (-1);
    }

    ssize_t len;
    if (dir == ATA_DIR_WRITE)
        len = write(handle, buf, nsectors * 512);
    else
        len = read(handle, buf, nsectors * 512);
    if (len == nsectors * 512)
        return (0); /* success */
    else {
        printf("read/write: atad device fd %d: %s\n", handle, strerror(errno));
        return (-1);
    }
}
