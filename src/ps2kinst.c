
#include "pfsd_client.h"
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static int install_kernel(const char *kernel);

static int usage(int es)
{
    fprintf(
        stderr,
        "usage: ps2kinst [options] kernel_file\n"
        "install linux kernel in system area.\n"
        "\n"
        "      --help         display this help and exit\n");
    return es;
}

static int show_help;

static const struct option long_options[] = {
    {
        "help",
        0,
        &show_help,
        1,
    },
    {
        NULL,
        0,
        NULL,
        0,
    },
};

int main(int ac, char **av)
{
    for (;;) {
        switch (getopt_long(ac, av, "h", long_options, NULL)) {
            case -1:
                if (optind >= ac)
                    return usage(1);
                if (show_help)
                    return usage(0);
                return install_kernel(av[optind]);
            case 0:
                continue;
            case 'h':
                show_help = 1;
                break;
            default:
                return usage(1);
        }
    }
}

static char buf[0x8000];
static int rfd;

static int install_kernel_PFSlib(const char *kernel)
{
    int r;
    int wfd;

    if (scepfsInit()) {
        fprintf(stderr, "PFSlib init failed.\n");
        return -2;
    }
    r = scepfsMount("pfs0:", "hdd0:__system", 0, NULL, 0);
    if (r < 0) {
        fprintf(stderr, "Mount failed.\n");
        return -3;
    }
    r = scepfsMkdir("pfs0:/p2lboot", 511);
    if (r < 0 && r != -17) {
        fprintf(stderr, "Mkdir failed.\n");
        return -4;
    }
    r = scepfsChdir("pfs0:/p2lboot");
    if (r < 0) {
        fprintf(stderr, "Chdir failed.\n");
        return -5;
    }
    wfd = scepfsOpen("pfs0:vmlinux", 1538, 511);
    if (wfd < 0) {
        fprintf(stderr, "Create failed.\n");
        return -6;
    }
    for (;;) {
        r = read(rfd, buf, sizeof(buf));
        if (r <= 0)
            break;
        if (scepfsWrite(wfd, buf, r) != r) {
            fprintf(stderr, "Write failed.\n");
            return -7;
        }
    }
    if (r < 0) {
        fprintf(stderr, "Read failed.\n");
        return -8;
    }
    close(rfd);
    scepfsClose(wfd);
    scepfsUmount("pfs0:");
    scepfsExit();
    fprintf(stderr, "Installation Succeed.\n");
    return 0;
}

static int install_kernel(const char *kernel)
{
    int r;
    int wfd;

    rfd = open(kernel, O_RDONLY);
    if (rfd < 0) {
        fprintf(stderr, "can't open kernel_file.\n");
        return -1;
    }
    if (scepfsdInit()) {
        fprintf(stderr, "use PFSlib instead\n");
        return install_kernel_PFSlib(kernel);
    }
    r = scepfsdMount("pfs0:", "hdd0:__system", 0, NULL, 0);
    if (r < 0) {
        fprintf(stderr, "Mount failed.\n");
        return -3;
    }
    r = scepfsdMkdir("pfs0:/p2lboot", 511);
    if (r < 0 && r != -17) {
        fprintf(stderr, "Mkdir failed.\n");
        return -4;
    }
    r = scepfsdChdir("pfs0:/p2lboot");
    if (r < 0) {
        fprintf(stderr, "Chdir failed.\n");
        return -5;
    }
    wfd = scepfsdOpen("pfs0:vmlinux", 1538, 511);
    if (wfd < 0) {
        fprintf(stderr, "Create failed.\n");
        return -6;
    }
    for (;;) {
        r = read(rfd, buf, sizeof(buf));
        if (r <= 0)
            break;
        if (scepfsdWrite(wfd, buf, r) != r) {
            fprintf(stderr, "Write failed.\n");
            return -7;
        }
    }
    if (r < 0) {
        fprintf(stderr, "Read failed.\n");
        return -8;
    }
    close(rfd);
    scepfsdClose(wfd);
    scepfsdUmount("pfs0:");
    scepfsdExit();
    fprintf(stderr, "Installation Succeed.\n");
    return 0;
}
