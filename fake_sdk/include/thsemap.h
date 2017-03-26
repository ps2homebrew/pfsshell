#pragma once

typedef struct
{
    u32 attr;
    u32 option;
    int initial;
    int max;
} iop_sema_t;

#define CreateSema(x) 0
#define DeleteSema(x) 0
#define WaitSema(x) 0
#define SignalSema(x) 0
