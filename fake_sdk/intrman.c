#include "types.h"
#include "dbg.h"
#include <stdlib.h>


/* DONE */
int
CpuDisableIntr (void)
{
  dbg_printf ("CpuDisableIntr ()\n");
  return (0);
}


int
CpuEnableIntr (void)
{
  dbg_printf ("CpuEnableIntr ()\n");
  return (0);
}


int
CpuSuspendIntr (int *state)
{
  dbg_printf ("CpuSuspendIntr ()\n");
  return (0);
}


int
CpuResumeIntr (int state)
{
  dbg_printf ("CpuResumeIntr ()\n");
  return (0);
}
