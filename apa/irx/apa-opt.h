#ifndef _APA_OPT_H
#define _APA_OPT_H

#define APA_PRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define APA_DRV_NAME "hdd"

#define APA_STAT_RETURN_PART_LBA 1
#define APA_FORMAT_MAKE_PARTITIONS 1
#define APA_FORMAT_LOCK_MBR 1

#define APA_MODVER_MAJOR 2
#define APA_MODVER_MINOR 1

#define _start apa_start

#endif
