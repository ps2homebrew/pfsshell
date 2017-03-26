#include <stdio.h>

int pfs_start(int argc, char *argv[]);

int _init_pfs(int argc, char *argv[])
{
    char *args[] =
        {
            "ps2pfs.irx",
            "-m", "1",
            "-o", "1",
            "-n", "10",
            NULL};
    int result = pfs_start(7, args);
    return (result);
}
