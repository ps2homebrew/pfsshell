#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "dbg.h"



/* DONE */

typedef int32_t s32;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef struct
{
  u32 devicePresent;
  u32 supportPACKET;
  u32 totalLBA;
  u32 securityStatus;
} t_shddInfo;

#define ATAD_MODE_READ 0x00
#define ATAD_MODE_WRITE 0x01


/* dd if=/dev/zero of=../hdd.img bs=1024 seek=49999999 count=1 */

static int handle = -1;

static unsigned long hdd_length = 0; /* in sectors */
char atad_device_path[256] = { "/home/bobi/p/pfs/hdd.img" };
void atad_close (void)
{
  if (handle != -1)
    close (handle), handle = -1;
}

void
init (void)
{
  handle = open (atad_device_path, O_RDWR);
  if (handle != -1)
    {
      off_t size = lseek (handle, 0, SEEK_END);
      if (size != (off_t) -1)
	hdd_length = (size - 511) / 512;
      else
	perror (atad_device_path), exit (1);
    }
  else
    perror (atad_device_path), exit (1);
}

t_shddInfo*
atadInit (u32 device)
{
  if (handle == -1)
    init ();

  static t_shddInfo info;
  if (device == 0)
    {
      info.devicePresent = 1;
      info.supportPACKET = 0;
      info.totalLBA = hdd_length;
      info.securityStatus = 0;
    }
  else
    {
      info.devicePresent = 0;
      info.supportPACKET = 0;
      info.totalLBA = 0;
      info.securityStatus = 0;
    }
  return (&info);
}

int
atadDmaTransfer (int device, void *buf, u32 lba, u32 size, u32 mode)
{
  if (handle == -1)
    init ();

  if (device != 0)
    {
      dbg_printf ("atadDmaTransfer: invalid device %d\n", device);
      return (-1);
    }

  off_t pos = lseek (handle, (off_t) lba * 512, SEEK_SET);
  if (pos == (off_t) -1)
    {
      dbg_printf ("lseek: %s: %s\n", atad_device_path, strerror (errno));
      return (-1);
    }

  ssize_t len;
  if (mode == ATAD_MODE_WRITE)
    len = write (handle, buf, size * 512);
  else
    len = read (handle, buf, size * 512);
  if (len == size * 512)
    return (0); /* success */
  else
    {
      dbg_printf ("read/write: %s: %s\n", atad_device_path, strerror (errno));
      return (-1);
    }
}


