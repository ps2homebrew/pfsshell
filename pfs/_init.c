#include <stdio.h>

int
_init_pfs (int argc, char *argv[])
{
  char *args[] =
    {
      "ps2pfs.irx",
      //"-m", "2",
      //"-o", "8",
      //"-n", "64",
      NULL
    };
  int _start(int argc, char *argv[]);
  int result = _start (1, args);
  return (result);
}
