#pragma once

#include <sysclib.h>

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

ata_devinfo_t *ata_get_devinfo(int device);
int ata_device_sector_io(int device, void *buf, u32 lba, u32 nsectors, int dir);

#define ata_device_sce_sec_unlock(x, y) 0
#define ata_device_idle(x, y) 0
#define ata_device_idle_immediate(x) 0
#define ata_device_sce_identify_drive(x, y) -1
#define ata_device_smart_get_status(x) 0
#define ata_device_smart_save_attr(x) 0
#define ata_device_flush_cache(x) 0
