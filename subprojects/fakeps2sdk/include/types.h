#pragma once

#include <sysclib.h>
#include <stddef.h>

#ifndef iomanX_struct
#define iomanX_struct
typedef struct
{
    /*00*/ unsigned int mode;
    /*04*/ unsigned int attr;
    /*08*/ unsigned int size;
    /*0c*/ unsigned char ctime[8];
    /*14*/ unsigned char atime[8];
    /*1c*/ unsigned char mtime[8];
    /*24*/ unsigned int hisize;
    /*28*/ unsigned int private_0; //Number of subs (main) / subpart number (sub)
    /*2c*/ unsigned int private_1;
    /*30*/ unsigned int private_2;
    /*34*/ unsigned int private_3;
    /*38*/ unsigned int private_4;
    /*3c*/ unsigned int private_5; /* Sector start.  */
} iox_stat_t;

typedef struct
{
    iox_stat_t stat;
    char name[256];
    unsigned int unknown;
} iox_dirent_t;
#endif
