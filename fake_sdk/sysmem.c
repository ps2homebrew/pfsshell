#include "sysmem.h"
#include <stdlib.h>


/* DONE */
void*
AllocSysMemory (int mode, int size, void *ptr)
{
  return (malloc (size));
}


int
FreeSysMemory (void *ptr)
{
  free (ptr);
  return (0);
}
