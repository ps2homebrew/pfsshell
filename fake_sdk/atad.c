#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "dbg.h"

#ifdef __APPLE__
#include <sys/disk.h>
#endif

typedef uint32_t u32;
typedef uint64_t u64;

/* These are used with the dir parameter of ata_device_sector_io().  */
#define ATA_DIR_READ 0
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
char atad_device_path[256] = {"hdd.img"};
void atad_close(void)
{
    if (handle != -1)
        close(handle), handle = -1;
}

void init(void)
{
    handle = open(atad_device_path, O_RDWR |
#ifdef O_BINARY
                                        O_BINARY
#else
                                        0
#endif
                  );
    if (handle != -1) {
#ifdef __APPLE__
        u64 size = 0, sector_count = 0;
        u32 sector_size = 0;
        if ( ioctl(handle, DKIOCGETBLOCKCOUNT, &sector_count) == 0 ) {
            ioctl(handle, DKIOCGETBLOCKSIZE, &sector_size);
            if ( sector_size != 512 )
                size = ((sector_count * sector_size) - 511) / 512;
            else
                size = sector_count;
            if ( (int64_t)size >= 0 )
                hdd_length = size;
            else
                perror(atad_device_path), exit(1);
        } else if ( errno == ENOTTY ) {
            /* Not a device. Fall back to lseek */
#endif
        off_t size = lseek(handle, 0, SEEK_END);
        if (size != (off_t)-1)
            hdd_length = (size - 511) / 512;
        else
            perror(atad_device_path), exit(1);
#ifdef __APPLE__
        }
#endif
    } else
        perror(atad_device_path), exit(1);
}

ata_devinfo_t *ata_get_devinfo(int device)
{
    if (handle == -1)
        init();

    static ata_devinfo_t info;
    if (device == 0) {
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
    if (handle == -1)
        init();

    if (device != 0) {
        dbg_printf("atadDmaTransfer: invalid device %d\n", device);
        return (-1);
    }

    off_t pos = lseek(handle, (off_t)lba * 512, SEEK_SET);
    if (pos == (off_t)-1) {
        dbg_printf("lseek: %s: %s\n", atad_device_path, strerror(errno));
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
        dbg_printf("read/write: %s: %s\n", atad_device_path, strerror(errno));
        return (-1);
    }
}
