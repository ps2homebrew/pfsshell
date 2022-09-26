
#include "types.h"
#include "thsemap.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define SEMA_COUNT 100
static sem_t *sem[SEMA_COUNT] = {NULL};

#if defined(__MACH__)
#define USE_NAMED_SEMAPHORES
#endif

int CreateSema(iop_sema_t *sema)
{
    size_t i = 0;
    while (i < SEMA_COUNT && sem[i] != NULL) {
        ++i;
    }
    if (i < SEMA_COUNT) {
#ifdef USE_NAMED_SEMAPHORES
        char sema_name_buf[NAME_MAX - 4];
        snprintf(sema_name_buf, sizeof(sema_name_buf), "fakeps2sdk%x_%i", getpid(), i);
        sem[i] = sem_open((const char *)sema_name_buf, O_CREAT, O_RDWR, 1);
#else
        sem[i] = (sem_t *)malloc(sizeof(sem_t));
#endif
        if (sem[i] != NULL) {
#ifdef USE_NAMED_SEMAPHORES
            sem_unlink((const char *)sema_name_buf);
            return i;
#else
            int retv = sem_init(sem[i], 0, 1);
            if (retv != -1) {
                return i;
            } else {
                printf("sem_init failed with %d\n", errno);
                free(sem[i]);
                sem[i] = NULL;
                return -1;
            }
#endif
        } else {
            printf("sem_open failed with %d\n", errno);
            return -1;
        }
    } else {
        printf("too many semaphores allocated\n");
        return -1;
    }
}

int DeleteSema(int semid)
{
    if (semid >= 0 && semid < SEMA_COUNT) {
        if (sem[semid] != NULL) {
#ifdef USE_NAMED_SEMAPHORES
            int retv = sem_close(sem[semid]);
#else
            int retv = sem_destroy(sem[semid]);
#endif
            if (retv == 0) {
#ifndef USE_NAMED_SEMAPHORES
                free(sem[semid]);
#endif
                sem[semid] = NULL;
                return 0;
            } else {
                printf("sem_close failed with %d\n", errno);
                return -1;
            }
        } else {
            printf("DeleteSema: already freed: %d\n", semid);
            return -1;
        }
    } else {
        printf("DeleteSema: invalid index: %d\n", semid);
        return -1;
    }
}

int WaitSema(int semid)
{
    if (semid >= 0 && semid < SEMA_COUNT) {
        if (sem[semid] != NULL) {
            return (sem_wait(sem[semid]));
        } else {
            printf("WaitSema: already freed: %d\n", semid);
            return (-1);
        }
    } else {
        printf("WaitSema: invalid index: %d\n", semid);
        return (-1);
    }
}

int SignalSema(int semid)
{
    if (semid >= 0 && semid < SEMA_COUNT) {
        if (sem[semid] != NULL) {
            return (sem_post(sem[semid]));
        } else {
            printf("SignalSema: already freed: %d\n", semid);
            return (-1);
        }
    } else {
        printf("SignalSema: invalid index: %d\n", semid);
        return (-1);
    }
}
