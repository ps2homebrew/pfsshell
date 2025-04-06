
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "pfsd_common.h"
#include "pfslib_compat.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

static void rpc_command_close(const pfsd_rpc_close_struct_t *pkt, int in_pid);
static void rpc_command_umount(const pfsd_rpc_umount_struct_t *pkt, int in_pid);

static int SemId;
static int ShmId;
static void *ShmPtr;

int StartUp_Daemon(void)
{
    SemId = semget(PFSD_SHARED_KEY, 1, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (SemId < 0) {
        perror("Can't create semaphore");
        return 1;
    }
    if (semctl(SemId, 0, SETVAL, 1) < 0) {
        perror("semctl");
        return 1;
    }
    ShmId = shmget(PFSD_SHARED_KEY, 0x20000, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (ShmId < 0) {
        perror("Can't create shared memory");
        return 1;
    }
    ShmPtr = shmat(ShmId, NULL, 0);
    return 0;
}

int CleanUp_Daemon(void)
{
    semctl(SemId, 0, IPC_RMID, NULL);
    shmdt(ShmPtr);
    shmctl(ShmId, IPC_RMID, NULL);
    unlink(PFSD_PATH);
    return 0;
}

typedef struct mountlist_item
{
    int m_pid;
    char *m_str_fsprefix;
    char *m_str_containerpath;
    char *m_str_devprefix;
} mountlist_item_t;

typedef struct openfile_item_
{
    int m_fd;
    int m_pid;
    int m_mountlist_index;
} openfile_item_t;

static int g_current_client_count;
static int g_openfile_count;
static int debug_mode;
static struct pollfd g_connection_pollfds[32];
static int SocketFd;
static openfile_item_t g_openfile_items[32];
static mountlist_item_t g_mountlist_items[32];

int find_free_mount_info(void)
{
    int i;

    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && g_mountlist_items[i].m_pid; i += 1)
        ;
    return i;
}

void umount_cleanup(int in_mountlist_index)
{
    g_mountlist_items[in_mountlist_index].m_pid = 0;
    free((void *)g_mountlist_items[in_mountlist_index].m_str_fsprefix);
    free((void *)g_mountlist_items[in_mountlist_index].m_str_containerpath);
    free((void *)g_mountlist_items[in_mountlist_index].m_str_devprefix);
}

void output_statistics(void)
{
    int mount_count;
    int i;

    mount_count = 0;
    printf("MountList:");
    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])); i += 1) {
        if (g_mountlist_items[i].m_pid) {
            if (!mount_count)
                printf("\n");
            printf(" pid%2d %s(%s) --> %s\n", g_mountlist_items[i].m_pid, g_mountlist_items[i].m_str_fsprefix, g_mountlist_items[i].m_str_containerpath, g_mountlist_items[i].m_str_devprefix);
            mount_count += 1;
        }
    }
    if (!mount_count)
        printf(" No partition mounted.\n");
    if (!g_openfile_count) {
        printf("OpenFile: No file inuse.\n");
        return;
    }
    printf("OpenFile:\n");
    for (i = 0; i < g_openfile_count; i += 1) {
        printf(" pid%2d fd%2d ", g_openfile_items[i].m_pid, g_openfile_items[i].m_fd);
        if (g_openfile_items[i].m_mountlist_index < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && g_mountlist_items[g_openfile_items[i].m_mountlist_index].m_pid)
            printf("(%s,%s)\n", g_mountlist_items[g_openfile_items[i].m_mountlist_index].m_str_fsprefix, g_mountlist_items[g_openfile_items[i].m_mountlist_index].m_str_devprefix);
        else
            printf(": Invalid MountPoint???\n");
    }
}

char *rpc_remap_name(char *in_name, int in_pid)
{
    int i;

    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (g_mountlist_items[i].m_pid != in_pid || strncmp(in_name, g_mountlist_items[i].m_str_fsprefix, strlen(g_mountlist_items[i].m_str_fsprefix))); i += 1)
        ;
    if (i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) {
        memmove(&in_name[strlen(g_mountlist_items[i].m_str_containerpath)], &in_name[strlen(g_mountlist_items[i].m_str_fsprefix)], strlen(in_name) - (strlen(g_mountlist_items[i].m_str_fsprefix) - 1));
        memcpy(in_name, g_mountlist_items[i].m_str_containerpath, strlen(g_mountlist_items[i].m_str_containerpath));
    } else {
        for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])); i += 1) {
            if (g_mountlist_items[i].m_pid && !strncmp(in_name, g_mountlist_items[i].m_str_containerpath, strlen(g_mountlist_items[i].m_str_containerpath))) {
                printf("pid%d requesting invalid filename '%s'\n", in_pid, in_name);
                *in_name = '\x00';
            }
        }
    }
    return in_name;
}

static void rpc_common_open(int in_fd, int in_pid, const char *in_filename)
{
    int i;

    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (g_mountlist_items[i].m_pid != in_pid || strncmp(in_filename, g_mountlist_items[i].m_str_fsprefix, strlen(g_mountlist_items[i].m_str_fsprefix))); i += 1)
        ;
    if (i == (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && strncmp(in_filename, "hdd", 3u))
        printf("Invalid filename '%s', fd%d pid%d.\n", in_filename, in_fd, in_pid);
    g_openfile_items[g_openfile_count].m_fd = in_fd;
    g_openfile_items[g_openfile_count].m_pid = in_pid;
    g_openfile_items[g_openfile_count].m_mountlist_index = i;
    g_openfile_count += 1;
}

static void rpc_common_close(int in_fd, int in_pid)
{
    int i;

    for (i = 0; i < g_openfile_count && (g_openfile_items[i].m_fd != in_fd || g_openfile_items[i].m_pid != in_pid); i += 1)
        ;
    for (; i < g_openfile_count - 1; i += 1)
        memcpy(&g_openfile_items[i], &g_openfile_items[i + 1], sizeof(openfile_item_t));
    g_openfile_count -= 1;
}

int rpc_remap_fd(int in_fd, int in_pid)
{
    int i;

    for (i = 0; i < g_openfile_count; i += 1) {
        if (g_openfile_items[i].m_fd == in_fd && g_openfile_items[i].m_pid == in_pid)
            return in_fd;
    }
    printf("pid%d requesting invalid fd%d access!\n", in_pid, in_fd);
    return -1;
}

int Connect_Daemon(void)
{
    int s;
    struct sockaddr_un sa;

    s = socket(AF_UNIX, SOCK_STREAM, PF_UNSPEC);
    if (s < 0) {
        perror("socket");
        return -1;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path, PFSD_PATH);
    unlink(PFSD_PATH);
    if (bind(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind");
        return -1;
    }
    chmod(PFSD_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
    if (listen(s, 5) < 0) {
        perror("listen");
        return -1;
    }
    signal(SIGPIPE, SIG_IGN);
    return s;
}

pid_t get_pid_from_socket_fd(int in_socket_fd)
{
    struct ucred ucr;
    socklen_t optlen;

    optlen = sizeof(ucr);
    if (getsockopt(in_socket_fd, SOL_SOCKET, SO_PEERCRED, &ucr, &optlen)) {
        perror("getsockopt SO_PEERCRED");
        return -1;
    }
    return ucr.pid;
}

typedef struct my_cap_user_header_
{
    int m_version;
    pid_t m_pid;
} my_cap_user_header_t;

typedef struct my_cap_user_data_
{
    int m_effective;
    // cppcheck-suppress unusedStructMember
    int m_permitted;
    // cppcheck-suppress unusedStructMember
    int m_inheritable;
} my_cap_user_data_t;

void add_new_client(void)
{
    int s;
    struct sockaddr_un sa;
    my_cap_user_header_t cuh;
    my_cap_user_data_t cud;
    socklen_t sl;

    sl = sizeof(sa);
    s = accept(SocketFd, &sa, &sl);
    if (s < 0) {
        perror("accept");
        return;
    }
    if (g_current_client_count >= sizeof(g_connection_pollfds) / sizeof(g_connection_pollfds[0])) {
        printf("Too many clients.\n");
        close(s);
        return;
    }
    // _LINUX_CAPABILITY_VERSION_1
    cuh.m_version = 0x19980330;
    cuh.m_pid = get_pid_from_socket_fd(s);
    if (cuh.m_pid == -1 || syscall(SYS_capget, &cuh, &cud)) {
        perror("capget");
        close(s);
        return;
    }
    // check CAP_SYS_RAWIO
    if (!(cud.m_effective & 0x20000)) {
        printf("Client does not have capability. disconnecting.\n");
        close(s);
        return;
    }
    g_connection_pollfds[g_current_client_count].fd = s;
    g_connection_pollfds[g_current_client_count].events = 1;
    g_current_client_count += 1;
    g_connection_pollfds[g_current_client_count].fd = SocketFd;
    g_connection_pollfds[g_current_client_count].events = 1;
}

void cleanup_client(int in_client_id)
{
    int outputflag;
    int i;

    outputflag = 1;
    for (i = 0; i < g_openfile_count; i += 1) {
        if (g_openfile_items[i].m_pid == g_connection_pollfds[in_client_id].fd) {
            pfsd_rpc_close_struct_t rpc_struct;

            rpc_struct.m_fd = g_openfile_items[i].m_fd;
            if (outputflag) {
                outputflag = 0;
                printf("%d terminated abnormally.\n", g_openfile_items[i].m_pid);
            }
            printf("Cleanup: Close %d\n", rpc_struct.m_fd);
            rpc_command_close(&rpc_struct, g_openfile_items[i].m_pid);
            i -= 1;
        }
    }
    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])); i += 1) {
        if (g_mountlist_items[i].m_pid == g_connection_pollfds[in_client_id].fd) {
            pfsd_rpc_umount_struct_t rpc_struct;

            strcpy(rpc_struct.m_fsname, g_mountlist_items[i].m_str_fsprefix);
            if (outputflag) {
                outputflag = 0;
                printf("%d terminated abnormally.\n", g_mountlist_items[i].m_pid);
            }
            printf("Cleanup: Unmount %s\n", rpc_struct.m_fsname);
            rpc_command_umount(&rpc_struct, g_mountlist_items[i].m_pid);
        }
    }
    if (!outputflag && debug_mode)
        output_statistics();
    close(g_connection_pollfds[in_client_id].fd);
    for (i = in_client_id; i < g_current_client_count; i += 1)
        memcpy(&g_connection_pollfds[i], &g_connection_pollfds[i + 1], sizeof(struct pollfd));
    g_current_client_count -= 1;
}

void rpc_command_open(pfsd_rpc_open_struct_t *pkt, int in_pid)
{
    char name_buf[1200];
    int rpcret;

    strcpy(name_buf, pkt->m_name);
    // ioman has a file descriptor limit, but check here also
    rpcret = (g_openfile_count < (sizeof(g_openfile_items) / sizeof(g_openfile_items[0]))) ? scepfsOpen(rpc_remap_name(name_buf, in_pid), pkt->m_flags, pkt->m_mode) : -24;
    write(in_pid, &rpcret, sizeof(rpcret));
    if (rpcret >= 0)
        rpc_common_open(rpcret, in_pid, pkt->m_name);
}

static void rpc_command_close(const pfsd_rpc_close_struct_t *pkt, int in_pid)
{
    int rpcret;

    rpcret = scepfsClose(rpc_remap_fd(pkt->m_fd, in_pid));
    if (!rpcret)
        rpc_common_close(pkt->m_fd, in_pid);
    write(in_pid, &rpcret, sizeof(rpcret));
}

static void rpc_command_dopen(const pfsd_rpc_dopen_struct_t *pkt, int in_pid)
{
    char name_buf[1200];
    int rpcret;

    strcpy(name_buf, pkt->m_name);
    // ioman has a file descriptor limit, but check here also
    rpcret = (g_openfile_count < (sizeof(g_openfile_items) / sizeof(g_openfile_items[0]))) ? scepfsDopen(rpc_remap_name(name_buf, in_pid)) : -24;
    write(in_pid, &rpcret, sizeof(rpcret));
    if (rpcret >= 0)
        rpc_common_open(rpcret, in_pid, pkt->m_name);
}

static void rpc_command_dclose(const pfsd_rpc_dclose_struct_t *pkt, int in_pid)
{
    int rpcret;

    rpcret = scepfsDclose(rpc_remap_fd(pkt->m_fd, in_pid));
    if (!rpcret)
        rpc_common_close(pkt->m_fd, in_pid);
    write(in_pid, &rpcret, sizeof(rpcret));
}

static void rpc_command_mount(const pfsd_rpc_mount_struct_t *pkt, int in_pid)
{
    int rpcret;
    int free_mount_info;
    char fsname_buf[1200];
    char devname_buf[1200];
    char *comma_ptr;

    strcpy(fsname_buf, pkt->m_fsname);
    strcpy(devname_buf, pkt->m_devname);
    comma_ptr = strchr(devname_buf, ',');
    if (comma_ptr)
        *comma_ptr = '\x00';
    rpcret = -24;
    free_mount_info = find_free_mount_info();
    if (free_mount_info < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) {
        int i;

        rpcret = -16;
        for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (g_mountlist_items[i].m_pid != in_pid || strcmp(g_mountlist_items[i].m_str_fsprefix, fsname_buf)); i += 1)
            ;
        if (i >= (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) {
            rpcret = scepfsMount(fsname_buf, devname_buf, pkt->m_flag, pkt->m_arg_useshmptr ? ShmPtr : NULL, pkt->m_arglen);
            if (rpcret == -16) {
                for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (!g_mountlist_items[i].m_pid || strcmp(g_mountlist_items[i].m_str_devprefix, devname_buf)); i += 1)
                    ;
                if (i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) {
                    rpcret = 0;
                    strcpy(fsname_buf, g_mountlist_items[i].m_str_containerpath);
                } else {
                    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && rpcret == -16; i += 1) {
                        sprintf(fsname_buf, "pfs%d:", i);
                        rpcret = scepfsMount(fsname_buf, devname_buf, pkt->m_flag, pkt->m_arg_useshmptr ? ShmPtr : NULL, pkt->m_arglen);
                    }
                }
            }
        }
    }
    if (!rpcret) {
        g_mountlist_items[free_mount_info].m_pid = in_pid;
        g_mountlist_items[free_mount_info].m_str_fsprefix = strdup(pkt->m_fsname);
        g_mountlist_items[free_mount_info].m_str_containerpath = strdup(fsname_buf);
        g_mountlist_items[free_mount_info].m_str_devprefix = strdup(devname_buf);
    }
    write(in_pid, &rpcret, sizeof(rpcret));
}

static void rpc_command_umount(const pfsd_rpc_umount_struct_t *pkt, int in_pid)
{
    int i;
    int j;
    int k;
    int rpcret;

    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (g_mountlist_items[i].m_pid != in_pid || strcmp(pkt->m_fsname, g_mountlist_items[i].m_str_fsprefix)); i += 1)
        ;
    if (i == (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) {
        printf("pid%d %s Unmount requested but not mounted.\n", in_pid, pkt->m_fsname);
        rpcret = -19;
        write(in_pid, &rpcret, sizeof(rpcret));
        return;
    }
    for (j = 0; j < g_openfile_count; j += 1) {
        if (g_openfile_items[j].m_mountlist_index == i) {
            printf(
                "pid%d %s %s Umount requested but there are still opened files.\n",
                in_pid,
                pkt->m_fsname,
                g_mountlist_items[i].m_str_devprefix);
            output_statistics();
            rpcret = -16;
            write(in_pid, &rpcret, sizeof(rpcret));
            return;
        }
    }
    for (k = 0; k < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && i == k && (!g_mountlist_items[k].m_pid || strcmp(g_mountlist_items[i].m_str_containerpath, g_mountlist_items[k].m_str_containerpath)); k += 1)
        ;
    if (k < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && debug_mode)
        printf(
            "pid%d %s %s Unmount requested but another process is using.\n",
            in_pid,
            pkt->m_fsname,
            g_mountlist_items[i].m_str_devprefix);
    rpcret = (k >= (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) ? scepfsUmount(g_mountlist_items[i].m_str_containerpath) : 0;
    write(in_pid, &rpcret, sizeof(rpcret));
    if (!rpcret)
        umount_cleanup(i);
    if (k < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && debug_mode)
        output_statistics();
}

void rpc_command_getmountpoint(pfsd_rpc_getmountpoint_struct_t *pkt, int in_pid)
{
    int i;
    int rpcret;
    char path_buf[1200];
    char *comma_ptr;

    strcpy(path_buf, pkt->m_path);
    comma_ptr = strchr(path_buf, ',');
    if (comma_ptr)
        *comma_ptr = '\x00';
    for (i = 0; i < (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0])) && (g_mountlist_items[i].m_pid != in_pid || strcmp(path_buf, g_mountlist_items[i].m_str_devprefix)); i += 1)
        ;
    strcpy((char *)ShmPtr, (i == (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) ? "" : g_mountlist_items[i].m_str_fsprefix);
    rpcret = (i == (sizeof(g_mountlist_items) / sizeof(g_mountlist_items[0]))) ? -19 : 0;
    write(in_pid, &rpcret, sizeof(rpcret));
}

void event_handler_func(int pfd)
{
    int fd;
    pfsd_rpc_common_struct_t rpc_struct_common;
    int rpcret;
    int64_t rpcret64;

    fd = g_connection_pollfds[pfd].fd;
    if (read(fd, &rpc_struct_common, sizeof(rpc_struct_common)) < 0) {
        perror("read");
        return;
    }
    switch (rpc_struct_common.m_command) {
        case PFSD_REQ_OPEN:
            rpc_command_open(&rpc_struct_common.m_opu.m_opi_open, fd);
            return;
        case PFSD_REQ_LSEEK:
            rpcret = scepfsLseek(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_lseek.m_fd, fd), rpc_struct_common.m_opu.m_opi_lseek.m_offset, rpc_struct_common.m_opu.m_opi_lseek.m_whence);
            break;
        case PFSD_REQ_LSEEK64:
            rpcret64 = scepfsLseek64(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_lseek64.m_fd, fd), rpc_struct_common.m_opu.m_opi_lseek64.m_offset, rpc_struct_common.m_opu.m_opi_lseek64.m_whence);
            write(fd, &rpcret64, sizeof(rpcret64));
            return;
        case PFSD_REQ_READ:
            rpcret = scepfsRead(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_read.m_fd, fd), rpc_struct_common.m_opu.m_opi_read.m_ptr_useshmptr ? ShmPtr : NULL, rpc_struct_common.m_opu.m_opi_read.m_size);
            break;
        case PFSD_REQ_WRITE:
            rpcret = scepfsWrite(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_write.m_fd, fd), rpc_struct_common.m_opu.m_opi_write.m_ptr_useshmptr ? ShmPtr : NULL, rpc_struct_common.m_opu.m_opi_write.m_size);
            break;
        case PFSD_REQ_CLOSE:
            rpc_command_close(&rpc_struct_common.m_opu.m_opi_close, fd);
            return;
        case PFSD_REQ_DOPEN:
            rpc_command_dopen(&rpc_struct_common.m_opu.m_opi_dopen, fd);
            return;
        case PFSD_REQ_DREAD:
            rpcret = scepfsDread(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_dread.m_fd, fd), rpc_struct_common.m_opu.m_opi_dread.m_iox_dirent_useshmptr ? ShmPtr : NULL);
            break;
        case PFSD_REQ_DCLOSE:
            rpc_command_dclose(&rpc_struct_common.m_opu.m_opi_dclose, fd);
            return;
        case PFSD_REQ_CHSTAT:
            rpcret = scepfsChstat(rpc_remap_name(rpc_struct_common.m_opu.m_opi_chstat.m_name, fd), rpc_struct_common.m_opu.m_opi_chstat.m_stat_useshmptr ? ShmPtr : NULL, rpc_struct_common.m_opu.m_opi_chstat.m_mask);
            break;
        case PFSD_REQ_GETSTAT:
            rpcret = scepfsGetstat(rpc_remap_name(rpc_struct_common.m_opu.m_opi_getstat.m_name, fd), rpc_struct_common.m_opu.m_opi_getstat.m_stat_useshmptr ? ShmPtr : NULL);
            break;
        case PFSD_REQ_MKDIR:
            rpcret = scepfsMkdir(rpc_remap_name(rpc_struct_common.m_opu.m_opi_mkdir.m_name, fd), rpc_struct_common.m_opu.m_opi_mkdir.m_mode);
            break;
        case PFSD_REQ_RMDIR:
            rpcret = scepfsRmdir(rpc_remap_name(rpc_struct_common.m_opu.m_opi_rmdir.m_name, fd));
            break;
        case PFSD_REQ_CHDIR:
            rpcret = scepfsChdir(rpc_remap_name(rpc_struct_common.m_opu.m_opi_chdir.m_name, fd));
            break;
        case PFSD_REQ_REMOVE:
            rpcret = scepfsRemove(rpc_remap_name(rpc_struct_common.m_opu.m_opi_remove.m_name, fd));
            break;
        case PFSD_REQ_RENAME:
            rpcret = scepfsRename(rpc_remap_name(rpc_struct_common.m_opu.m_opi_rename.m_old, fd), rpc_remap_name(rpc_struct_common.m_opu.m_opi_rename.m_new, fd));
            break;
        case PFSD_REQ_IOCTL:
            rpcret = scepfsIoctl(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_ioctl.m_fd, fd), rpc_struct_common.m_opu.m_opi_ioctl.m_cmd, rpc_struct_common.m_opu.m_opi_ioctl.m_arg_useshmptr ? ShmPtr : NULL);
            break;
        case PFSD_REQ_IOCTL2:
            rpcret = scepfsIoctl2(
                rpc_remap_fd(rpc_struct_common.m_opu.m_opi_ioctl2.m_fd, fd),
                rpc_struct_common.m_opu.m_opi_ioctl2.m_cmd,
                rpc_struct_common.m_opu.m_opi_ioctl2.m_arg_useshmptr ? ShmPtr : NULL,
                rpc_struct_common.m_opu.m_opi_ioctl2.m_arglen,
                rpc_struct_common.m_opu.m_opi_ioctl2.m_buf_useshmptr ? ShmPtr : NULL,
                rpc_struct_common.m_opu.m_opi_ioctl2.m_buflen);
            break;
        case PFSD_REQ_SYMLINK:
            rpcret = scepfsSymlink(rpc_remap_name(rpc_struct_common.m_opu.m_opi_symlink.m_old, fd), rpc_remap_name(rpc_struct_common.m_opu.m_opi_symlink.m_new, fd));
            break;
        case PFSD_REQ_READLINK:
            rpcret = scepfsReadlink(rpc_remap_name(rpc_struct_common.m_opu.m_opi_readlink.m_name, fd), rpc_struct_common.m_opu.m_opi_readlink.m_buf_useshmptr ? ShmPtr : NULL, rpc_struct_common.m_opu.m_opi_readlink.m_buflen);
            break;
        case PFSD_REQ_SYNC:
            rpcret = scepfsSync(rpc_remap_name(rpc_struct_common.m_opu.m_opi_sync.m_dev, fd), rpc_struct_common.m_opu.m_opi_sync.m_flag);
            break;
        case PFSD_REQ_FORMAT: {
            char blockdev_buf[1200];
            char *comma_ptr;

            strcpy(blockdev_buf, rpc_struct_common.m_opu.m_opi_format.m_blockdev);
            comma_ptr = strchr(blockdev_buf, ',');
            if (comma_ptr)
                *comma_ptr = '\x00';
            rpcret = scepfsFormat(rpc_struct_common.m_opu.m_opi_format.m_dev, blockdev_buf, rpc_struct_common.m_opu.m_opi_format.m_arg_useshmptr ? ShmPtr : NULL, rpc_struct_common.m_opu.m_opi_format.m_arglen);
            break;
        }
        case PFSD_REQ_MOUNT:
            rpc_command_mount(&rpc_struct_common.m_opu.m_opi_mount, fd);
            return;
        case PFSD_REQ_UMOUNT:
            rpc_command_umount(&rpc_struct_common.m_opu.m_opi_umount, fd);
            return;
        case PFSD_REQ_DEVCTL:
            rpcret = scepfsDevctl(
                rpc_remap_name(rpc_struct_common.m_opu.m_opi_devctl.m_name, fd),
                rpc_struct_common.m_opu.m_opi_devctl.m_cmd,
                rpc_struct_common.m_opu.m_opi_devctl.m_arg_useshmptr ? ShmPtr : NULL,
                rpc_struct_common.m_opu.m_opi_devctl.m_arglen,
                rpc_struct_common.m_opu.m_opi_devctl.m_buf_useshmptr ? ShmPtr : NULL,
                rpc_struct_common.m_opu.m_opi_devctl.m_buflen);
            break;
        case PFSD_REQ_SETREADAHEAD:
            rpcret = scepfsSetReadAhead(rpc_struct_common.m_opu.m_opi_setreadahead.m_enabled);
            break;
        case PFSD_REQ_GETREADAHEAD:
            rpcret = scepfsGetReadAhead();
            break;
        case PFSD_REQ_GETMOUNTPOINT:
            rpc_command_getmountpoint(&rpc_struct_common.m_opu.m_opi_getmountpoint, fd);
            break;
        case PFSD_REQ_IOCTL2_HIOCTRANSFER: {
            hddIoctl2Transfer_t xfer;

            xfer.sub = rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_sub;
            xfer.sector = rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_sector;
            xfer.size = rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_size;
            xfer.mode = rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_mode;
            xfer.buffer = rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_buffer_useshmptr ? ShmPtr : NULL;
            rpcret = scepfsIoctl2(rpc_remap_fd(rpc_struct_common.m_opu.m_opi_ioctl2_hioctransfer.m_fd, fd), 0x6832, &xfer, sizeof(xfer), NULL, 0);
            break;
        }
        default:
            printf("Not supported function %d.\n", rpc_struct_common.m_command);
            return;
    }
    write(fd, &rpcret, sizeof(rpcret));
}

int main(int argc, const char **argv, const char **envp)
{
    if (argc >= 2)
        debug_mode = 1;
    if (scepfsInit() < 0) {
        printf("PFSlib initialize failed.\n");
        return 1;
    }
    if (StartUp_Daemon())
        return 1;
    SocketFd = Connect_Daemon();
    if (SocketFd < 0)
        return 1;
    g_connection_pollfds[g_current_client_count].fd = SocketFd;
    g_connection_pollfds[g_current_client_count].events = 1;
    for (;;) {
        int flag;

        for (flag = 1; flag && poll(g_connection_pollfds, g_current_client_count + 1, -1) >= 0;) {
            if ((g_connection_pollfds[g_current_client_count].revents & POLLIN))
                add_new_client();
            else {
                int i;

                for (i = 0; i < g_current_client_count; i += 1) {
                    if ((g_connection_pollfds[i].revents & POLLHUP)) {
                        cleanup_client(i);
                        flag = 0;
                        break;
                    }
                    if ((g_connection_pollfds[i].revents & POLLIN))
                        event_handler_func(i);
                }
            }
        }
        if (errno != EINTR) {
            perror("poll");
            return 1;
        }
        if (debug_mode)
            break;
    }
    output_statistics();
    return CleanUp_Daemon();
}
