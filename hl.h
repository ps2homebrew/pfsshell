#pragma once

/* copy file, host to PFS */
int copyto(const char *mount_point, const char *dest, const char *src);

/* copy file, PFS to host */
int copyfrom(const char *mount_point, const char *src, const char *dest);

/* list files at PFS */
int list_dir_objects(int dh);
int ls(const char *mount_point, const char *path);

/* list HDD partitions */
int lspart(void);

/* create PFS onto an existing partition */
int mkfs(const char *mount_point);

/* create partition and (optionally) format it as PFS;
 * so far the only sizes supported are powers of 2 */
int mkpart(const char *mount_point, long size_in_mb, int format);

/* initialize PS2 HDD with APA partitioning and create common partitions
 * (__mbr, __common, __net, etc.); common partitions are not PFS-formatted */
int initialize(void);
