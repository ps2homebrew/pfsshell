
#include "pfsd_client.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/socket.h>

static int SemID;
static int ShmID;
static void *ShmPtr;
static int Cfd;
static size_t SharedMemorySize;

int Connect(void)
{
    struct sockaddr_un sa;

    Cfd = socket(AF_UNIX, SOCK_STREAM, PF_UNSPEC);
    if (Cfd < 0) {
        perror("socket");
        return -1;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    if (bind(Cfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind");
        return -1;
    }
    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path, PFSD_PATH);
    if (connect(Cfd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("connect");
        return -1;
    }
    return 0;
}

size_t GetShmSize(void)
{
    struct shmid_ds shminfo;

    shmctl(ShmID, IPC_STAT, &shminfo);
    return shminfo.shm_segsz;
}

int StartUp(void)
{
    if (Connect() < 0) {
        printf("Can't connect to pfsd.\n");
        return -1;
    }
    SemID = semget(PFSD_SHARED_KEY, 1, 0);
    if (SemID < 0) {
        perror("semget");
        return -2;
    }
    ShmID = shmget(PFSD_SHARED_KEY, 0, 0);
    if (ShmID < 0) {
        perror("shmget");
        return -3;
    }
    ShmPtr = shmat(ShmID, NULL, 0);
    SharedMemorySize = GetShmSize();
    if ((int)SharedMemorySize <= 0) {
        fprintf(stderr, "GetShmSize error.\n");
        return -5;
    }
    return 0;
}

void CleanUp(void)
{
    shmdt(ShmPtr);
    close(Cfd);
}

void RequireLock(void)
{
    struct sembuf sema_obj[1];

    sema_obj[0].sem_num = 0;
    sema_obj[0].sem_op = -1;
    sema_obj[0].sem_flg = SEM_UNDO;
    if (semop(SemID, &sema_obj[0], sizeof(sema_obj) / sizeof(sema_obj[0])) < 0)
        perror("semop require lock");
}

void ReleaseLock(void)
{
    struct sembuf sema_obj[1];

    sema_obj[0].sem_num = 0;
    sema_obj[0].sem_op = 1;
    sema_obj[0].sem_flg = SEM_UNDO;
    if (semop(SemID, &sema_obj[0], sizeof(sema_obj) / sizeof(sema_obj[0])) < 0)
        perror("semop release lock");
}

void SendRequest(int rpc_cmd, const void *rpc_buf, size_t rpc_buf_size)
{
    size_t buf_size;
    pfsd_rpc_common_struct_t *rpc_buf_p;

    buf_size = rpc_buf_size + 8;
    rpc_buf_p = malloc(buf_size);
    rpc_buf_p->m_command = rpc_cmd;
    memcpy(rpc_buf_p->m_opu.m_payload, rpc_buf, rpc_buf_size);
    if (write(Cfd, rpc_buf_p, buf_size) < 0)
        perror("Send msg failed.\n");
    free(rpc_buf_p);
}

int ReceiveReply(void)
{
    int rpcret;

    if (read(Cfd, &rpcret, sizeof(rpcret)) <= 0)
        perror("Receive msg failed.\n");
    return rpcret;
}

int ReceiveReplyLong(void)
{
    int rpcret;

    if (read(Cfd, &rpcret, sizeof(rpcret)) <= 0)
        perror("Receive msg failed.\n");
    return rpcret;
}

int64_t ReceiveReplyLongLong(void)
{
    int64_t rpcret64;

    if (read(Cfd, &rpcret64, sizeof(rpcret64)) <= 0)
        perror("Receive msg failed.\n");
    return rpcret64;
}

int scepfsdInit(void)
{
    return StartUp();
}

void scepfsdExit(void)
{
    CleanUp();
}

int scepfsdOpen(const char *name, int flags, int mode)
{
    int ret;
    pfsd_rpc_open_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_flags = flags;
    rpc_struct.m_mode = mode;
    RequireLock();
    SendRequest(PFSD_REQ_OPEN, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdLseek(int fd, int offset, int whence)
{
    int ret;
    pfsd_rpc_lseek_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    rpc_struct.m_offset = offset;
    rpc_struct.m_whence = whence;
    RequireLock();
    SendRequest(PFSD_REQ_LSEEK, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReplyLong();
    ReleaseLock();
    return ret;
}

int64_t scepfsdLseek64(int fd, int64_t offset, int whence)
{
    int64_t ret;
    pfsd_rpc_lseek64_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    rpc_struct.m_offset = offset;
    rpc_struct.m_whence = whence;
    RequireLock();
    SendRequest(PFSD_REQ_LSEEK64, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReplyLongLong();
    ReleaseLock();
    return ret;
}

int scepfsdRead(int fd, void *ptr, size_t size)
{
    int chunked_size;
    int result;
    int ret;
    pfsd_rpc_read_struct_t rpc_struct;

    for (chunked_size = 0; SharedMemorySize < (size - chunked_size); chunked_size += result) {
        result = scepfsdRead(fd, (char *)ptr + chunked_size, SharedMemorySize);
        if (result < 0)
            return result;
        if (result < (int)SharedMemorySize)
            return result + chunked_size;
    }
    rpc_struct.m_fd = fd;
    rpc_struct.m_ptr_useshmptr = !!ptr;
    rpc_struct.m_size = size - chunked_size;
    RequireLock();
    SendRequest(PFSD_REQ_READ, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    if (ptr)
        memcpy(ptr, ShmPtr, rpc_struct.m_size);
    ReleaseLock();
    return ret + chunked_size;
}

int scepfsdWrite(int fd, void *ptr, size_t size)
{
    int chunked_size;
    int result;
    int ret;
    pfsd_rpc_write_struct_t rpc_struct;

    for (chunked_size = 0; SharedMemorySize < (size - chunked_size); chunked_size += result) {
        result = scepfsdWrite(fd, (char *)ptr + chunked_size, SharedMemorySize);
        if (result < 0)
            return result;
        if (result < (int)SharedMemorySize)
            return result + chunked_size;
    }
    rpc_struct.m_fd = fd;
    rpc_struct.m_ptr_useshmptr = !!ptr;
    rpc_struct.m_size = size - chunked_size;
    RequireLock();
    if (ptr)
        memcpy(ShmPtr, ptr, rpc_struct.m_size);
    SendRequest(PFSD_REQ_WRITE, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret + chunked_size;
}

int scepfsdClose(int fd)
{
    int ret;
    pfsd_rpc_close_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    RequireLock();
    SendRequest(PFSD_REQ_CLOSE, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdDopen(const char *name)
{
    int ret;
    pfsd_rpc_dopen_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    RequireLock();
    SendRequest(PFSD_REQ_DOPEN, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdDread(int fd, iox_dirent_t *iox_dirent)
{
    int ret;
    pfsd_rpc_dread_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    rpc_struct.m_iox_dirent_useshmptr = !!iox_dirent;
    RequireLock();
    SendRequest(PFSD_REQ_DREAD, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    memcpy(iox_dirent, ShmPtr, sizeof(iox_dirent_t));
    ReleaseLock();
    return ret;
}

int scepfsdDclose(int fd)
{
    int ret;
    pfsd_rpc_dclose_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    RequireLock();
    SendRequest(PFSD_REQ_DCLOSE, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdChstat(const char *name, iox_stat_t *stat, int mask)
{
    int ret;
    pfsd_rpc_chstat_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_stat_useshmptr = !!stat;
    rpc_struct.m_mask = mask;
    RequireLock();
    if (stat)
        memcpy(ShmPtr, stat, sizeof(iox_stat_t));
    SendRequest(PFSD_REQ_CHSTAT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdGetstat(const char *name, iox_stat_t *stat)
{
    int ret;
    pfsd_rpc_getstat_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_stat_useshmptr = !!stat;
    RequireLock();
    SendRequest(PFSD_REQ_GETSTAT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    if (stat)
        memcpy(stat, ShmPtr, sizeof(iox_stat_t));
    ReleaseLock();
    return ret;
}

int scepfsdMkdir(const char *name, int mode)
{
    int ret;
    pfsd_rpc_mkdir_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_mode = mode;
    RequireLock();
    SendRequest(PFSD_REQ_MKDIR, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdRmdir(const char *name)
{
    int ret;
    pfsd_rpc_rmdir_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    RequireLock();
    SendRequest(PFSD_REQ_RMDIR, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdChdir(const char *name)
{
    int ret;
    pfsd_rpc_chdir_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    RequireLock();
    SendRequest(PFSD_REQ_CHDIR, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdRemove(const char *name)
{
    int ret;
    pfsd_rpc_remove_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    RequireLock();
    SendRequest(PFSD_REQ_REMOVE, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdRename(const char *old, const char *new_)
{
    int ret;
    pfsd_rpc_rename_struct_t rpc_struct;

    strcpy(rpc_struct.m_old, old);
    strcpy(rpc_struct.m_new, new_);
    RequireLock();
    SendRequest(PFSD_REQ_RENAME, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdIoctl(int fd, int cmd, void *arg)
{
    int ret;
    pfsd_rpc_ioctl_struct_t rpc_struct;

    rpc_struct.m_fd = fd;
    rpc_struct.m_cmd = cmd;
    rpc_struct.m_arg_useshmptr = !!arg;
    RequireLock();
    if (arg)
        memcpy(ShmPtr, arg, 1024);
    SendRequest(PFSD_REQ_IOCTL, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdIoctl2(int fd, int command, void *arg, size_t arglen, void *buf, size_t buflen)
{
    int ret;

    if (command == 0x6832) {
        pfsd_rpc_ioctl2_hioctransfer_struct_t rpc_struct;
        hddIoctl2Transfer_t *xfer;

        xfer = arg;
        rpc_struct.m_fd = fd;
        rpc_struct.m_size = xfer->size;
        rpc_struct.m_buffer_useshmptr = !!xfer->buffer;
        rpc_struct.m_sub = xfer->sub;
        rpc_struct.m_mode = xfer->mode;
        rpc_struct.m_sector = xfer->sector;
        if (!xfer->buffer || rpc_struct.m_size > 0x100)
            return -22;
        RequireLock();
        memcpy(ShmPtr, xfer->buffer, rpc_struct.m_size << 9);
        SendRequest(PFSD_REQ_IOCTL2_HIOCTRANSFER, &rpc_struct, sizeof(rpc_struct));
        ret = ReceiveReply();
        memcpy(xfer->buffer, ShmPtr, rpc_struct.m_size << 9);
    } else {
        pfsd_rpc_ioctl2_struct_t rpc_struct;

        rpc_struct.m_fd = fd;
        rpc_struct.m_cmd = command;
        rpc_struct.m_arg_useshmptr = !!arg;
        rpc_struct.m_arglen = arglen;
        rpc_struct.m_buf_useshmptr = !!buf;
        rpc_struct.m_buflen = buflen;
        RequireLock();
        if (arg && arglen)
            memcpy(ShmPtr, arg, arglen);
        SendRequest(PFSD_REQ_IOCTL2, &rpc_struct, sizeof(rpc_struct));
        ret = ReceiveReply();
        if (buf && buflen)
            memcpy(buf, ShmPtr, buflen);
    }
    ReleaseLock();
    return ret;
}

int scepfsdSymlink(const char *old, const char *new_)
{
    int ret;
    pfsd_rpc_symlink_struct_t rpc_struct;

    strcpy(rpc_struct.m_old, old);
    strcpy(rpc_struct.m_new, new_);
    RequireLock();
    SendRequest(PFSD_REQ_SYMLINK, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdReadlink(const char *name, char *buf, size_t buflen)
{
    int ret;
    pfsd_rpc_readlink_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_buf_useshmptr = !!buf;
    rpc_struct.m_buflen = buflen;
    RequireLock();
    SendRequest(PFSD_REQ_READLINK, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    if (buf && buflen)
        memcpy(buf, ShmPtr, buflen);
    ReleaseLock();
    return ret;
}

int scepfsdSync(const char *dev, int flag)
{
    int ret;
    pfsd_rpc_sync_struct_t rpc_struct;

    strcpy(rpc_struct.m_dev, dev);
    rpc_struct.m_flag = flag;
    RequireLock();
    SendRequest(PFSD_REQ_SYNC, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdFormat(const char *dev, const char *blockdev, void *arg, size_t arglen)
{
    int ret;
    pfsd_rpc_format_struct_t rpc_struct;

    strcpy(rpc_struct.m_dev, dev);
    strcpy(rpc_struct.m_blockdev, blockdev);
    rpc_struct.m_arg_useshmptr = !!arg;
    rpc_struct.m_arglen = arglen;
    RequireLock();
    if (arg && arglen)
        memcpy(ShmPtr, arg, arglen);
    SendRequest(PFSD_REQ_FORMAT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdMount(const char *fsname, const char *devname, int flag, void *arg, size_t arglen)
{
    int ret;
    pfsd_rpc_mount_struct_t rpc_struct;

    strcpy(rpc_struct.m_fsname, fsname);
    strcpy(rpc_struct.m_devname, devname);
    rpc_struct.m_flag = flag;
    rpc_struct.m_arg_useshmptr = !!arg;
    rpc_struct.m_arglen = arglen;
    RequireLock();
    if (arg && arglen)
        memcpy(ShmPtr, arg, arglen);
    SendRequest(PFSD_REQ_MOUNT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdUmount(const char *fsname)
{
    int ret;
    pfsd_rpc_umount_struct_t rpc_struct;

    strcpy(rpc_struct.m_fsname, fsname);
    RequireLock();
    SendRequest(PFSD_REQ_UMOUNT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdDevctl(const char *name, int cmd, const void *arg, size_t arglen, void *buf, size_t buflen)
{
    int ret;
    pfsd_rpc_devctl_struct_t rpc_struct;

    strcpy(rpc_struct.m_name, name);
    rpc_struct.m_cmd = cmd;
    rpc_struct.m_arg_useshmptr = !!arg;
    rpc_struct.m_arglen = arglen;
    rpc_struct.m_buf_useshmptr = !!buf;
    rpc_struct.m_buflen = buflen;
    RequireLock();
    if (arg && arglen)
        memcpy(ShmPtr, arg, arglen);
    SendRequest(PFSD_REQ_DEVCTL, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    if (buf && buflen)
        memcpy(buf, ShmPtr, buflen);
    ReleaseLock();
    return ret;
}

int scepfsdSetReadAhead(int enabled)
{
    int ret;
    pfsd_rpc_setreadahead_struct_t rpc_struct;

    rpc_struct.m_enabled = enabled;
    RequireLock();
    SendRequest(PFSD_REQ_SETREADAHEAD, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdGetReadAhead(void)
{
    int ret;

    RequireLock();
    SendRequest(PFSD_REQ_GETREADAHEAD, NULL, 0);
    ret = ReceiveReply();
    ReleaseLock();
    return ret;
}

int scepfsdGetMountPoint(const char *in_oldmap, char *out_newmap)
{
    int ret;
    pfsd_rpc_getmountpoint_struct_t rpc_struct;

    strcpy(rpc_struct.m_path, in_oldmap);
    RequireLock();
    SendRequest(PFSD_REQ_GETMOUNTPOINT, &rpc_struct, sizeof(rpc_struct));
    ret = ReceiveReply();
    if (out_newmap)
        strcpy(out_newmap, (const char *)ShmPtr);
    ReleaseLock();
    return ret;
}
