#include <stdio.h>


int
_init_apa (int argc, char *argv[])
{
  char *args[] =
    {
      "ps2hdd.irx",
      NULL
    };
  int apa_start (int argc, char **argv);
  int result = apa_start (1, args);
  return (result);
}
