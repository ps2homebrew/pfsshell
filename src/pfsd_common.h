#pragma once

#include <stdint.h>

#define PFSD_PATH       "/tmp/pfsd"
#define PFSD_SHARED_KEY 0x7C724E1D

typedef struct pfsd_rpc_open_struct_
{
    char m_name[1024];
    int m_flags;
    int m_mode;
} pfsd_rpc_open_struct_t;

typedef struct pfsd_rpc_lseek_struct_
{
    int m_fd;
    int m_offset;
    int m_whence;
} pfsd_rpc_lseek_struct_t;

typedef struct pfsd_rpc_lseek64_struct_
{
    int m_fd;
    int m_pad1;
    int64_t m_offset;
    int m_whence;
    int m_pad2;
} pfsd_rpc_lseek64_struct_t;

typedef struct pfsd_rpc_read_struct_
{
    int m_fd;
    int m_ptr_useshmptr;
    int m_size;
} pfsd_rpc_read_struct_t;

typedef struct pfsd_rpc_write_struct_
{
    int m_fd;
    int m_ptr_useshmptr;
    int m_size;
} pfsd_rpc_write_struct_t;

typedef struct pfsd_rpc_close_struct_
{
    int m_fd;
} pfsd_rpc_close_struct_t;

typedef struct pfsd_rpc_dopen_struct_
{
    char m_name[1024];
} pfsd_rpc_dopen_struct_t;

typedef struct pfsd_rpc_dread_struct_
{
    int m_fd;
    int m_iox_dirent_useshmptr;
} pfsd_rpc_dread_struct_t;

typedef struct pfsd_rpc_dclose_struct_
{
    int m_fd;
} pfsd_rpc_dclose_struct_t;

typedef struct pfsd_rpc_chstat_struct_
{
    char m_name[1024];
    int m_stat_useshmptr;
    int m_mask;
} pfsd_rpc_chstat_struct_t;

typedef struct pfsd_rpc_getstat_struct_
{
    char m_name[1024];
    int m_stat_useshmptr;
} pfsd_rpc_getstat_struct_t;

typedef struct pfsd_rpc_mkdir_struct_
{
    char m_name[1024];
    int m_mode;
} pfsd_rpc_mkdir_struct_t;

typedef struct pfsd_rpc_rmdir_struct_
{
    char m_name[1024];
} pfsd_rpc_rmdir_struct_t;

typedef struct pfsd_rpc_chdir_struct_
{
    char m_name[1024];
} pfsd_rpc_chdir_struct_t;

typedef struct pfsd_rpc_remove_struct_
{
    char m_name[1024];
} pfsd_rpc_remove_struct_t;

typedef struct pfsd_rpc_rename_struct_
{
    char m_old[1024];
    char m_new[1024];
} pfsd_rpc_rename_struct_t;

typedef struct pfsd_rpc_ioctl_struct_
{
    int m_fd;
    int m_cmd;
    int m_arg_useshmptr;
} pfsd_rpc_ioctl_struct_t;

typedef struct pfsd_rpc_ioctl2_struct_
{
    int m_fd;
    int m_cmd;
    int m_arg_useshmptr;
    int m_arglen;
    int m_buf_useshmptr;
    int m_buflen;
} pfsd_rpc_ioctl2_struct_t;

typedef struct pfsd_rpc_symlink_struct_
{
    char m_old[1024];
    char m_new[1024];
} pfsd_rpc_symlink_struct_t;

typedef struct pfsd_rpc_readlink_struct_
{
    char m_name[1024];
    int m_buf_useshmptr;
    int m_buflen;
} pfsd_rpc_readlink_struct_t;

typedef struct pfsd_rpc_sync_struct_
{
    char m_dev[1024];
    int m_flag;
} pfsd_rpc_sync_struct_t;

typedef struct pfsd_rpc_format_struct_
{
    char m_dev[1024];
    char m_blockdev[1024];
    int m_arg_useshmptr;
    int m_arglen;
} pfsd_rpc_format_struct_t;

typedef struct pfsd_rpc_mount_struct_
{
    char m_fsname[1024];
    char m_devname[1024];
    int m_flag;
    int m_arg_useshmptr;
    int m_arglen;
} pfsd_rpc_mount_struct_t;

typedef struct pfsd_rpc_umount_struct_
{
    char m_fsname[1024];
} pfsd_rpc_umount_struct_t;

typedef struct pfsd_rpc_devctl_struct_
{
    char m_name[1024];
    int m_cmd;
    int m_arg_useshmptr;
    int m_arglen;
    int m_buf_useshmptr;
    int m_buflen;
} pfsd_rpc_devctl_struct_t;

typedef struct pfsd_rpc_setreadahead_struct_
{
    int m_enabled;
} pfsd_rpc_setreadahead_struct_t;

typedef struct pfsd_rpc_getmountpoint_struct_
{
    char m_path[1024];
} pfsd_rpc_getmountpoint_struct_t;

typedef struct pfsd_rpc_ioctl2_hioctransfer_struct_
{
    int m_fd;
    unsigned int m_sub;
    unsigned int m_sector;
    unsigned int m_size;
    unsigned int m_mode;
    int m_buffer_useshmptr;
} pfsd_rpc_ioctl2_hioctransfer_struct_t;

typedef struct hddIoctl2Transfer_
{
    unsigned int sub;
    unsigned int sector;
    unsigned int size;
    unsigned int mode;
    void *buffer;
} hddIoctl2Transfer_t;

typedef struct pfsd_rpc_common_struct_
{
    int m_unused;
    int m_command;
    union pfsd_rpc_common_struct_opu_
    {
        pfsd_rpc_open_struct_t m_opi_open;
        pfsd_rpc_lseek_struct_t m_opi_lseek;
        pfsd_rpc_lseek64_struct_t m_opi_lseek64;
        pfsd_rpc_read_struct_t m_opi_read;
        pfsd_rpc_write_struct_t m_opi_write;
        pfsd_rpc_close_struct_t m_opi_close;
        pfsd_rpc_dopen_struct_t m_opi_dopen;
        pfsd_rpc_dread_struct_t m_opi_dread;
        pfsd_rpc_dclose_struct_t m_opi_dclose;
        pfsd_rpc_chstat_struct_t m_opi_chstat;
        pfsd_rpc_getstat_struct_t m_opi_getstat;
        pfsd_rpc_mkdir_struct_t m_opi_mkdir;
        pfsd_rpc_rmdir_struct_t m_opi_rmdir;
        pfsd_rpc_chdir_struct_t m_opi_chdir;
        pfsd_rpc_remove_struct_t m_opi_remove;
        pfsd_rpc_rename_struct_t m_opi_rename;
        pfsd_rpc_ioctl_struct_t m_opi_ioctl;
        pfsd_rpc_ioctl2_struct_t m_opi_ioctl2;
        pfsd_rpc_symlink_struct_t m_opi_symlink;
        pfsd_rpc_readlink_struct_t m_opi_readlink;
        pfsd_rpc_sync_struct_t m_opi_sync;
        pfsd_rpc_format_struct_t m_opi_format;
        pfsd_rpc_mount_struct_t m_opi_mount;
        pfsd_rpc_umount_struct_t m_opi_umount;
        pfsd_rpc_devctl_struct_t m_opi_devctl;
        pfsd_rpc_setreadahead_struct_t m_opi_setreadahead;
        pfsd_rpc_getmountpoint_struct_t m_opi_getmountpoint;
        pfsd_rpc_ioctl2_hioctransfer_struct_t m_opi_ioctl2_hioctransfer;
        char m_payload[0];
    } m_opu;
} pfsd_rpc_common_struct_t;

enum pfsd_req_enum_ {
    PFSD_REQ_OPEN = 0,
    PFSD_REQ_LSEEK = 1,
    PFSD_REQ_LSEEK64 = 2,
    PFSD_REQ_READ = 3,
    PFSD_REQ_WRITE = 4,
    PFSD_REQ_CLOSE = 5,
    PFSD_REQ_DOPEN = 6,
    PFSD_REQ_DREAD = 7,
    PFSD_REQ_DCLOSE = 8,
    PFSD_REQ_CHSTAT = 9,
    PFSD_REQ_GETSTAT = 10,
    PFSD_REQ_MKDIR = 11,
    PFSD_REQ_RMDIR = 12,
    PFSD_REQ_CHDIR = 13,
    PFSD_REQ_REMOVE = 14,
    PFSD_REQ_RENAME = 15,
    PFSD_REQ_IOCTL = 16,
    PFSD_REQ_IOCTL2 = 17,
    PFSD_REQ_SYMLINK = 18,
    PFSD_REQ_READLINK = 19,
    PFSD_REQ_SYNC = 20,
    PFSD_REQ_FORMAT = 21,
    PFSD_REQ_MOUNT = 22,
    PFSD_REQ_UMOUNT = 23,
    PFSD_REQ_DEVCTL = 24,
    PFSD_REQ_SETREADAHEAD = 25,
    PFSD_REQ_GETREADAHEAD = 26,
    PFSD_REQ_GETMOUNTPOINT = 27,
    PFSD_REQ_IOCTL2_HIOCTRANSFER = 28,
};
