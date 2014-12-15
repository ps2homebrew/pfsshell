#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "dbg.h"

#include "dict.h"
#include "hio.h"
#include "retcodes.h"


/* DONE */

#if 1 /* enable 64-bit file I/O */
#  define open open64
#  define lseek lseek64
#  define off_t off64_t
#endif

typedef int32_t s32;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

typedef struct
{
  u32 devicePresent;
  u32 supportPACKET;
  u32 totalLBA;
  u32 securityStatus;
} t_shddInfo;

#define ATAD_MODE_READ 0x00
#define ATAD_MODE_WRITE 0x01

#define USE_HIO


/* dd if=/dev/zero of=../hdd.img bs=1024 seek=49999999 count=1 */
#if defined (USE_HIO)
static hio_t *hio = NULL;
#else
static int handle = -1;
#endif
static unsigned long hdd_length = 0; /* in sectors */
char atad_device_path[256] = { "/home/bobi/p/pfs/hdd.img" };
void atad_close (void)
{
#if defined (USE_HIO)
  if (hio != NULL)
    hio->close (hio), hio = NULL;
#else
  if (handle != -1)
    close (handle), handle = -1;
#endif
}

#if defined (USE_HIO)
void
init (void)
{
  dict_t *dict = dict_alloc ();
  int result = hio_probe (dict, atad_device_path, &hio);
  if (result == RET_OK)
    {
      printf ("hio: %s: opened.\n", atad_device_path);

      u_int32_t size_in_kb = 0;
      result = hio->stat (hio, &size_in_kb);
      if (result == RET_OK)
	{
	  hdd_length = size_in_kb * 2;
	  printf ("hio: %lu sectors.\n", hdd_length);
	}
      else
	{
	  hio->close (hio);
	  printf ("hio: %s: stat failed with %d.\n",
		  atad_device_path, result);
	  exit (1);
	}
    }
  else
    {
      perror (atad_device_path);
      printf ("hio: %s: open failed with %d.\n", atad_device_path, result);
      exit (1);
    }
}
#else
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
#endif /* USE_HIO defined? */


t_shddInfo*
atadInit (u32 device)
{
#if defined (USE_HIO)
  if (hio == NULL)
#else
  if (handle == -1)
#endif
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


#if defined (USE_HIO)
int
atadDmaTransfer (int device, void *buf, u32 lba, u32 size, u32 mode)
{
  if (hio == NULL)
    init ();

  if (device != 0)
    {
      dbg_printf ("atadDmaTransfer: invalid device %d\n", device);
      return (-1);
    }

  int result;

  u_int32_t len = 0;
  if (mode == ATAD_MODE_WRITE)
    result = hio->write (hio, lba, size, buf, &len);
  else
    result = hio->read (hio, lba, size, buf, &len);
  if (len == size * 512)
    return (0); /* success */
  else
    {
      dbg_printf ("read/write: %d: %s\n", result);
      return (-1);
    }
}
#else
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
#endif /* USE_HIO defined? */


int
atadSceUnlock (s32 device, u8 *key)
{
  dbg_printf ("atadSceUnlock (%d, %.10s)\n", device, key);
  return (0);
}


int
atadIdle (s32 device, u8 time)
{
  dbg_printf ("atadIdle (%d, %d)\n", device, time);
  return (0);
}


int
atadSceIdentifyDrive (s32 device, u16 *buffer)
{
  dbg_printf ("atadSceIdentifyDrive (%d, %p)\n", device, buffer);
  return (-1);
}


int
atadGetStatus (s32 device)
{
  dbg_printf ("atadGetStatus (%d)\n", device);
  return (0);
}


int
atadUpdateAttrib (s32 device)
{
  dbg_printf ("atadUpdateAttrib (%d)\n", device);
  return (0);
}


int
atadFlushCache (s32 device)
{
  dbg_printf ("atadFlushCache (%d)\n", device);
  return (0);
}
