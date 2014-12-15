#include "irx.h"


#if defined (_BUILD_WIN32)
char*
index (const char *p, char ch)
{
  return (strchr (p, ch));
}
#endif


int
hook_ioman (void)
{
  return (0);
}

int
unhook_ioman (void)
{
  return (0);
}

int
RegisterLibraryEntries (struct irx_export_table *p)
{
  return (0);
}

int
FlushIcache (void)
{
  return (0);
}


int
ioctl2 (int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen)
{
  int iomanx_ioctl2(int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
  return (iomanx_ioctl2 (fd, cmd, arg, arglen, buf, buflen));
}
