#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "util.h"
#include "iomanX_port.h"
#include "hl.h"

#ifdef __WIN32
#define NEED_GETLINE
#endif

typedef struct
{
    int setup;
    char mount_point[256];
    int mount;
    char path[256]; /* "/" when root; not `/'-terminated */
} context_t;

enum requirements {
    no_req = 0,
    need_device = 1,    /* device should be selected */
    need_no_device = 2, /* device should not be selected */
    need_mount = 4,     /* partition should be mounted */
    need_no_mount = 8,  /* partition should not be mounted */
};
static int check_requirements(context_t *ctx, enum requirements req)
{
    if ((req & need_device) && !ctx->setup) {
        fputs("(!) No device selected; use `device' command.\n", stderr);
        return (0);
    }
    if ((req & need_no_device) && ctx->setup) {
        fputs("(!) Device already selected; restart first.\n", stderr);
        return (0);
    }
    if ((req & need_mount) && !ctx->mount) {
        fputs("(!) No partition mounted; use `mount' command.\n", stderr);
        return (0);
    }
    if ((req & need_no_mount) && ctx->mount) {
        fputs("(!) Partition already mounted; use `umount' first.\n", stderr);
        return (0);
    }
    return (1);
}

#ifdef NEED_GETLINE
size_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while (c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif

static int shell_loop(FILE *in, FILE *out, FILE *err,
                      int (*process)(void *data, int argc, char *argv[]),
                      void *data)
{

    char prompt[256] = {"> "};
    char *line = NULL;
    size_t len = 0;

    while (true) {
        fputs(prompt, err);
        getline(&line, &len, stdin);
        if (!strncmp(line, "exit", 4) || !strncmp(line, "quit", 4) || !strncmp(line, "bye", 3)) {
            break;
        }
        char *tokens[256];
        size_t count;
        int result = parse_line(line, &tokens, &count);
        if (result >= 0) {
            errno = 0; /* reset */
            result = (*process)(data, count, tokens);
            if (result != 0) { /* probably operation has failed */
                fprintf(err, "(!) Exit code is %d", result);
                if (errno != 0)
                    fprintf(err, "; errno %d (%s).\n", errno, strerror(-errno));
                else
                    fprintf(err, ".\n");
            }
        } else
            fputs("(!) Unable to parse command line.\n", err);

        /* update prompt */
        context_t *ctx = (context_t *)data;
        if (ctx->setup) {
            if (ctx->mount)
                sprintf(prompt, "%s:%s# ", strchr(ctx->mount_point, ':') + 1, ctx->path);
            else
                sprintf(prompt, "# ");
        }
    }
    return (0);
}

static int do_lcd(context_t *ctx, int argc, char *argv[])
{
    if (argc == 1) { /* display local working dir */
        char buf[256];
        if (getcwd(buf, sizeof(buf))) {
            printf("%s\n", buf);
            return (0);
        }
        return (-1);
    } else
        /* chdir would set errno on error */
        return (chdir(argv[1]));
}

static int do_device(context_t *ctx, int argc, char *argv[])
{
    /* where (image of) PS2 HDD is; in fake_sdk/atad.c */
    extern char atad_device_path[256];
    strcpy(atad_device_path, argv[1]);

    /* mandatory */
    int result = _init_apa(0, NULL);
    if (result < 0) {
        fprintf(stderr, "(!) init_apa: failed with %d (%s)\n", result,
                strerror(-result));
        exit(1);
    }

    /* mandatory */
    result = _init_pfs(0, NULL);
    if (result < 0) {
        fprintf(stderr, "(!) init_pfs: failed with %d (%s)\n", result,
                strerror(-result));
        exit(1);
    }

    /* mandatory */
    result = _init_hdlfs(0, NULL);
    if (result < 0) {
        fprintf(stderr, "(!) init_hdlfs: failed with %d (%s)\n", result,
                strerror(-result));
        exit(1);
    }
    ctx->setup = 1;
    return (0);
}

static int do_initialize(context_t *ctx, int argc, char *argv[])
{
    if (argc == 1) {
        fprintf(stdout, "Use: initialize yes to create APA partitioning.\n");
        return (0);
    } else {
        int result = iomanx_format("hdd0:", NULL, NULL, 0);
        if (result < 0)
            fprintf(stderr, "(!) format: %s.\n", strerror(-result));
        return (result);
    }
}

static int do_mkfs(context_t *ctx, int argc, char *argv[])
{
#define PFS_ZONE_SIZE 8192
#define PFS_FRAGMENT 0x00000000
    int format_arg[] = {PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT};

    char tmp[256];
    strcpy(tmp, "hdd0:");
    strcat(tmp, argv[1]);
    int result =
        iomanx_format("pfs:", tmp, (void *)&format_arg, sizeof(format_arg));
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_mkpart(context_t *ctx, int arg, char *argv[])
{
    int size_in_mb = strtoul(argv[2], NULL, 10);
    if (size_in_mb == 0) {
        fprintf(stderr, "%s: invalid partition size.\n", argv[2]);
        return (-1);
    }

    char tmp[256];
    if (size_in_mb >= 1024)
        sprintf(tmp, "hdd0:%s,,,%dG,PFS", argv[1], size_in_mb / 1024);
    else
        sprintf(tmp, "hdd0:%s,,,%dM,PFS", argv[1], size_in_mb);
    int result = iomanx_open(tmp, IOMANX_O_RDWR | IOMANX_O_CREAT);
    if (result >= 0)
        (void)iomanx_close(result), result = 0;
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", argv[1], strerror(-result));
    return (result);
}

static int do_ls(context_t *ctx, int argc, char *argv[])
{
    if (!ctx->mount) {         /* no mount: list partitions */
        int result = lspart(); /* in hl.c */
        if (result < 0)
            fprintf(stderr, "(!) lspart: %s.\n", strerror(-result));
        return (result);
    } else { /* list objects in current directory */
        char dir_path[256];
        strcpy(dir_path, "pfs0:");
        strcat(dir_path, ctx->path);
        int dh = iomanx_dopen(dir_path);
        if (dh >= 0) {            /* dopen successful */
            list_dir_objects(dh); /* in hl.c */
            (void)iomanx_close(dh);
            return (0);
        } else {
            fprintf(stderr, "(!) ls: %s\n", strerror(-dh));
            return (dh);
        }
    }
}

static int do_mount(context_t *ctx, int argc, char *argv[])
{
    strcpy(ctx->mount_point, "hdd0:");
    strcat(ctx->mount_point, argv[1]);
    int result = iomanx_mount("pfs0:", ctx->mount_point, 0, NULL, 0);
    if (result >= 0) {
        ctx->mount = 1;
        return (0);
    } else {
        fprintf(stderr, "(!) %s: %s.\n", ctx->mount_point, strerror(-result));
        return (result);
    }
}

static int do_umount(context_t *ctx, int argc, char *argv[])
{
    int result = iomanx_umount("pfs0:");
    if (result >= 0) {
        ctx->mount = 0;
        return (0);
    } else {
        fprintf(stderr, "(!) umount: %s.\n", strerror(-result));
        return (result);
    }
}

static int do_pwd(context_t *ctx, int argc, char *argv[])
{
    fprintf(stdout, "%s\n", ctx->path);
    return (0);
}

static int do_cd(context_t *ctx, int argc, char *argv[])
{
    char backup[256];
    strcpy(backup, ctx->path);
    if (!strcmp(argv[1], "/")) /* root */
        strcpy(ctx->path, "/");
    else if (!strcmp(argv[1], "..") || !strcmp(argv[1], "../")) { /* parent */
        char *slash = strrchr(ctx->path, '/');
        if (slash != ctx->path)
            *slash = '\0';
        else
            ctx->path[1] = '\0'; /* return to root dir */
    } else if (argv[1][0] == '/')
        /* absolute */
        strcpy(ctx->path, argv[1]);
    else { /* relative */
        if (strlen(ctx->path) > 1)
            strcat(ctx->path, "/");
        strcat(ctx->path, argv[1]);
    }

    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    int result = iomanx_chdir(tmp);
    if (result < 0) { /* error */
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
        strcpy(ctx->path, backup); /* rollback */
    }
    return (result);
}

static int do_mkdir(context_t *ctx, int argc, char *argv[])
{
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);
    int result = iomanx_mkdir(tmp, 0777);
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_rmdir(context_t *ctx, int argc, char *argv[])
{
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);
    int result = iomanx_rmdir(tmp);
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_get(context_t *ctx, int argc, char *argv[])
{
    int result = 0;
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);

    int in = iomanx_open(tmp, IOMANX_O_RDONLY);
    if (in >= 0) {
        int out = open(argv[1], O_CREAT | O_WRONLY |
#ifdef O_BINARY
                                    O_BINARY
#else
                                    0
#endif
                       ,
                       0664);
        if (out != -1) {
            char buf[4096];
            ssize_t len;
            while ((len = iomanx_read(in, buf, sizeof(buf))) > 0) {
                result = write(out, buf, len);
                if (result != len) {
                    perror(argv[1]);
                    result = -1;
                    break;
                }
            }
            result = close(out);
        } else
            perror(argv[1]), result = -1;
        iomanx_close(in);
    } else
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-in)), result = in;
    return (result);
}

static int do_put(context_t *ctx, int argc, char *argv[])
{
    int result = 0;
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);

    int in = open(argv[1], O_RDONLY |
#ifdef O_BINARY
                               O_BINARY
#else
                               0
#endif
    );
    if (in != -1) {
        int out = iomanx_open(tmp, IOMANX_O_WRONLY | IOMANX_O_CREAT, 0666);
        if (out >= 0) {
            char buf[8 * 4096]; /* bigger buffer performs better via network */
            ssize_t len;
            while ((len = read(in, buf, sizeof(buf))) > 0) {
                result = iomanx_write(out, buf, len);
                if (result != len) {
                    if (result < 0)
                        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
                    else
                        fprintf(stderr, "(!) %s: wrote %d bytes instead of %lu.\n", tmp,
                                result, len);
                    result = -1;
                    break;
                }
            }
            result = iomanx_close(out);
        } else
            fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-out)), result = out;
        (void)close(in);
    } else
        perror(argv[1]), result = -1;
    return (result);
}

static int do_rm(context_t *ctx, int argc, char *argv[])
{
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);
    int result = iomanx_remove(tmp);
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_rename(context_t *ctx, int argc, char *argv[])
{
    char tmp[256];
    strcpy(tmp, "pfs0:");
    strcat(tmp, ctx->path);
    if (tmp[strlen(tmp) - 1] != '/')
        strcat(tmp, "/");
    strcat(tmp, argv[1]);
    int result = iomanx_rename(tmp, argv[2]);
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_rmpart(context_t *ctx, int argc, char *argv[])
{
    char tmp[256];
    strcpy(tmp, "hdd0:");
    strcat(tmp, argv[1]);
    int result = iomanx_remove(tmp);
    if (result < 0)
        fprintf(stderr, "(!) %s: %s.\n", tmp, strerror(-result));
    return (result);
}

static int do_help(context_t *ctx, int argc, char *argv[])
{
    fputs(
        "~Command List~\n"
        "lcd [path] - print/change the local working directory\n"
        "device <device> - use this PS2 HDD;\n"
        "initialize - blank and create APA on a new PS2 HDD (destructive);\n"
        "mkpart <part_name> <size> - create a new partition;\n"
        "\tSize must be a power of 2;\n"
        "mkfs <part_name> - blank and create PFS on a new partition "
        "(destructive);\n"
        "mount <part_name> - mount a partition;\n"
        "umount - un-mount a partition;\n"
        "ls - no mount: list partitions; mount: list files/dirs;\n"
        "mkdir <dir_name> - create a new directory;\n"
        "rmdir <dir_name> - delete an existing directory;\n"
        "pwd - print current PS2 HDD directory;\n"
        "cd <dir_name> - change directory;\n"
        "get <file_name> - copy file from PS2 HDD to current dir;\n"
        "put <file_name> - copy file from current dir to PS2 HDD;\n"
        "\tfile name must not contain a path;"
        "rm <file_name> - delete a file;\n"
        "rename <curr_name> <new_name> - rename a file/dir.\n"
        "rmpart <part_name> - remove partition (destructive).\n"
        "exit - exits the program. (Do this before you unplug your HDD)\n",
        stderr);
    return (0);
}

static int exec(void *data, int argc, char *argv[])
{
    if (argc == 0)
        return (0);

    struct command
    {
        const char *name;
        int args;
        enum requirements req;
        int (*process)(context_t *ctx, int argc, char *argv[]);
    };
    static const struct command CMD[] = {
        {"lcd", 0, no_req, &do_lcd},
        {"lcd", 1, no_req, &do_lcd},
        {"device", 1, need_no_device, &do_device},
        {"initialize", 0, need_device + need_no_mount, &do_initialize},
        {"initialize", 1, need_device + need_no_mount, &do_initialize},
        {"mkpart", 2, need_device, &do_mkpart},
        {"mkfs", 1, need_device, &do_mkfs},
        {"mount", 1, need_device + need_no_mount, &do_mount},
        {"umount", 0, need_device + need_mount, &do_umount},
        {"ls", 0, need_device, &do_ls},
        {"mkdir", 1, need_device + need_mount, &do_mkdir},
        {"rmdir", 1, need_device + need_mount, &do_rmdir},
        {"pwd", 0, need_device + need_mount, &do_pwd},
        {"cd", 1, need_device + need_mount, &do_cd},
        {"get", 1, need_device + need_mount, &do_get},
        {"put", 1, need_device + need_mount, &do_put},
        {"rm", 1, need_device + need_mount, &do_rm},
        {"rmpart", 1, need_device, &do_rmpart},
        {"rename", 2, need_device + need_mount, &do_rename},
        {"help", 0, no_req, &do_help},
    };
    static const size_t CMD_COUNT = sizeof(CMD) / sizeof(CMD[0]);

    context_t *ctx = (context_t *)data;
    int i;
    for (i = 0; i < CMD_COUNT; ++i) {
        const struct command *cmd = CMD + i;
        if (!strcmp(argv[0], cmd->name) && cmd->args == argc - 1) {
            if (check_requirements(ctx, cmd->req))
                return ((*cmd->process)(ctx, argc, argv));
            else
                return (0);
        }
    }

    fprintf(stderr, "(!) %s: unknown command or bad number of arguments.\n",
            argv[0]);
    return (0);
}

int shell(FILE *in, FILE *out, FILE *err)
{
    extern void atad_close(void); /* fake_sdk/atad.c */
    context_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    strcpy(ctx.path, "/");

    fputs(
        "pfsshell for POSIX systems\n"
        "https://github.com/uyjulian/pfsshell\n"
        "\n"
        "This program uses pfs, apa, iomanX, \n"
        "code from ps2sdk (https://github.com/ps2dev/ps2sdk)\n"
        "\n"
        "Type \"help\" for a list of commands.\n"
        "\n",
        stderr);

    int result = shell_loop(in, out, err, &exec, &ctx);
    if (ctx.mount)
        do_umount(&ctx, 0, NULL);
    atad_close();
    return (result);
}
