#include "thsemap.h"
#include "dbg.h"
#if defined (_BUILD_UNIX)
#  include <semaphore.h>
#else
#  include "sema.h" /* from hdl_dump */
#endif
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#define SEMA_COUNT 100 
static sem_t *sem[SEMA_COUNT] = { NULL };
static const char *nam[SEMA_COUNT] = { NULL };

/* DONE */
int
CreateSema (iop_sema_t *sema)
{
  /* scan for avail */
  size_t i = 0;
  while (i < SEMA_COUNT && nam[i] != NULL)
    ++i;
  if (i < SEMA_COUNT)
    {
      sem_t *retv;
      char stss[32];
      sprintf(stss, "abc%zu", i);
      nam[i] = stss;
  if (nam[i] != NULL)
	{
    errno = 0;
	  retv = sem_open (nam[i], O_CREAT, S_IRWXU, 1);
	  if (!(retv == SEM_FAILED))
    {
      sem[i] = retv;
	    return (i); /* success */
    }
	  else
	    {
        dbg_printf ("CreateSema: err: %d\n", errno);
	     free (sem[i]), nam[i] = NULL;
	      return (-1); /* sem_open failed */
	    }
	}
      else
	return (-1); /* out-of-memory */
    }
  else
    return (-1); /* too many semaphores allocated */
}


int
DeleteSema (int semid)
{
  if (semid >= 0 && semid < SEMA_COUNT)
    {
      if (nam[semid] != NULL)
	{
	  int retv = sem_unlink (nam[semid]);
	  if (retv == 0)
	    {
	      free (sem[semid]), nam[semid] = NULL;
	      return (0);
	    }
	  else
	    {
	      dbg_printf ("sem_unlink failed with %d\n", errno);
	      return (-1);
	    }
	}
      else
	{
	  dbg_printf ("DeleteSema: already freed: %d\n", semid);
	  return (-1);
	}
    }
  else
    {
      dbg_printf ("DeleteSema: invalid index: %d\n", semid);
      return (-1);
    }
}


int
WaitSema (int semid)
{
  if (semid >= 0 && semid < SEMA_COUNT)
    {
      if (nam[semid] != NULL)
	{
        int retv = sem_wait (sem[semid]);
    if (retv == 0)
      {
        return (0);
      }
    else
      {
        dbg_printf ("sem_wait failed with %d\n", errno);
        return (-1);
      }
	}
      else
	{
	  dbg_printf ("WaitSema: already freed: %d\n", semid);
	  return (-1);
	}
    }
  else
    {
      dbg_printf ("WaitSema: invalid index: %d\n", semid);
      return (-1);
    }
}


int
SignalSema (int semid)
{
  if (semid >= 0 && semid < SEMA_COUNT)
    {
      if (nam[semid] != NULL)
	{
            int retv = sem_post (sem[semid]);
    if (retv == 0)
      {
        return (0);
      }
    else
      {
        dbg_printf ("sem_post failed with %d\n", errno);
        return (-1);
      }
	}
      else
	{
	  dbg_printf ("SignalSema: already freed: %d\n", semid);
	  return (-1);
	}
    }
  else
    {
      dbg_printf ("SignalSema: invalid index: %d\n", semid);
      return (-1);
    }
}
