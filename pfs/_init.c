#include <stdio.h>

int pfs_start(int argc, char *argv[]);

int
_init_pfs (int argc, char *argv[])
{
  char *args[] =
    {
      "ps2pfs.irx",
      "-m", "2",
      "-o", "8",
      "-n", "64",
      NULL
    };
  int result = pfs_start (7, args);
  return (result);
}
