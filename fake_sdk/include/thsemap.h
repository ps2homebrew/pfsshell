
#pragma once

typedef struct {
	u32	attr;
	u32	option;
	int	initial;
	int	max;
} iop_sema_t;

int CreateSema(iop_sema_t *sema);
int DeleteSema(int semid);
int WaitSema(int semid);
int SignalSema(int semid);

static inline int CreateMutex(int state)
{
	iop_sema_t sema;
	sema.attr = 0;
	sema.option = 0;
	sema.initial = state;
	sema.max = 1;
	return CreateSema(&sema);
}
