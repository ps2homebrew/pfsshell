#include <stdio.h>

int apa_start(int argc, char **argv);

int _init_apa(int argc, char *argv[])
{
    char *args[] =
        {
            "ps2hdd.irx",
            NULL};
    int pass_argc = 1;
    char **pass_argv = args;
    if (argv != NULL) {
        pass_argc = argc;
        pass_argv = argv;
    }
    int result = apa_start(pass_argc, pass_argv);
    return (result);
}
