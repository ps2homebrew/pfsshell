#pragma once

typedef struct
{
    u32 attr;
    u32 option;
    int initial;
    int max;
} iop_sema_t;

int CreateSema(iop_sema_t *sema);
int DeleteSema(int semid);
int WaitSema(int semid);
int SignalSema(int semid);
