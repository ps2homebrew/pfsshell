#ifndef _PFS_OPT_H
#define _PFS_OPT_H

#define PFS_PRINTF(format, ...) printf(format, ##__VA_ARGS__)
#define PFS_DRV_NAME "pfs"

#include <types.h>
#define _start pfs_start

#endif
