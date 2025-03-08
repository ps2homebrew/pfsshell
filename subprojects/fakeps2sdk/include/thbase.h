#pragma once

#include <tamtypes.h>

#define TH_C 0x02000000

typedef struct _iop_thread
{
    u32 attr;
    u32 option;
    void (*thread)(void *);
    u32 stacksize;
    u32 priority;
} iop_thread_t;

typedef struct _iop_sys_clock
{
    u32 lo, hi;
} iop_sys_clock_t;

int CreateThread(iop_thread_t *thread);
int StartThread(int thid, void *arg);

int CheckThreadStack(void);

int GetSystemTime(iop_sys_clock_t *sys_clock);

void SysClock2USec(iop_sys_clock_t *sys_clock, u32 *sec, u32 *usec);
