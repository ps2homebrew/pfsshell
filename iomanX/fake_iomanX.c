#include <sysclib.h>

int
ioctl2 (int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen)
{
  int iomanx_ioctl2(int fd, int cmd, void *arg, size_t arglen, void *buf, size_t buflen);
  return (iomanx_ioctl2 (fd, cmd, arg, arglen, buf, buflen));
}
