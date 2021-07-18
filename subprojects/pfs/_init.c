#include <stdio.h>

int pfs_start(int argc, char *argv[]);

int _init_pfs(int argc, char *argv[])
{
    int result = pfs_start(argc, argv);
    return (result);
}
