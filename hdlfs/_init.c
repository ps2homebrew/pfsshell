#include <stdio.h>

int hdlfs_start(int argc, char *argv[]);

int _init_hdlfs(int argc, char *argv[])
{
    char *args[] =
        {
            NULL};
    int result = hdlfs_start(0, args);
    return (result);
}
