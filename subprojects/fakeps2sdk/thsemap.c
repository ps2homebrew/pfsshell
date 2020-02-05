
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

int CreateSema(iop_sema_t *sema)
{
    size_t i = 0;
    while (i < SEMA_COUNT && sem[i] != NULL)
        ++i;
    if (i < SEMA_COUNT) {
        char sema_name_buf[NAME_MAX - 4];
        snprintf(sema_name_buf, sizeof(sema_name_buf), "fakeps2sdk%x_%i", getpid(), i);
        sem[i] = sem_open((const char *)sema_name_buf, O_CREAT);
        if (sem[i] != NULL) {
            sem_unlink((const char *)sema_name_buf);
            return (i);
        } else {
            printf("sem_open failed with %d\n", errno);
            return -1;
        }
    } else {
        printf("too many semaphores allocated\n");
        return (-1);
    }
}

int DeleteSema(int semid)
{
    if (semid >= 0 && semid < SEMA_COUNT) {
        if (sem[semid] != NULL) {
            int retv = sem_close(sem[semid]);
            if (retv == 0) {
                sem[semid] = NULL;
                return (0);
            } else {
                printf("sem_close failed with %d\n", errno);
                return (-1);
            }
        } else {
            printf("DeleteSema: already freed: %d\n", semid);
            return (-1);
        }
    } else {
        printf("DeleteSema: invalid index: %d\n", semid);
        return (-1);
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
