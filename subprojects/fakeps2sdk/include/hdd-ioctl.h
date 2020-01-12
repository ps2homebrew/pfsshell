#pragma once

//apa

// Partition format/types (as returned via the mode field for getstat/dread)
#define APA_TYPE_FREE 0x0000
#define APA_TYPE_MBR 0x0001 // Master Boot Record
#define APA_TYPE_EXT2SWAP 0x0082
#define APA_TYPE_EXT2 0x0083
#define APA_TYPE_PFS 0x0100
#define APA_TYPE_CFS 0x0101
#define APA_TYPE_HDL 0x1337

#define APA_IDMAX 32
#define APA_MAXSUB 64 // Maximum # of sub-partitions
#define APA_PASSMAX 8
#define APA_FLAG_SUB 0x0001 // Sub-partition status for partitions (attr field)

//
// IOCTL2 commands
//
#define HIOCADDSUB 0x6801
#define HIOCDELSUB 0x6802
#define HIOCNSUB 0x6803
#define HIOCFLUSH 0x6804

// Arbitrarily-named commands
#define HIOCTRANSFER 0x6832     // Used by PFS.IRX to read/write data
#define HIOCGETSIZE 0x6833      // For main(0)/subs(1+)
#define HIOCSETPARTERROR 0x6834 // Set (sector of a partition) that has an error
#define HIOCGETPARTERROR 0x6835 // Get (sector of a partition) that has an error

//HDLFS addition
#define HIOCGETPARTSTART 0x6836 // Get the sector number of the first sector of the partition.

// I/O direction
#define APA_IO_MODE_READ 0x00
#define APA_IO_MODE_WRITE 0x01

// structs for IOCTL2 commands
typedef struct
{
    u32 sub; // main(0)/subs(1+) to read/write
    u32 sector;
    u32 size; // in sectors
    u32 mode; // ATAD_MODE_READ/ATAD_MODE_WRITE.....
    void *buffer;
} hddIoctl2Transfer_t;

//
// DEVCTL commands
//
// 'H' set
#define HDIOC_MAXSECTOR 0x4801   // Maximum partition size (in sectors)
#define HDIOC_TOTALSECTOR 0x4802 // Capacity of the disk (in sectors)
#define HDIOC_IDLE 0x4803
#define HDIOC_FLUSH 0x4804
#define HDIOC_SWAPTMP 0x4805
#define HDIOC_DEV9OFF 0x4806
#define HDIOC_STATUS 0x4807
#define HDIOC_FORMATVER 0x4808
#define HDIOC_SMARTSTAT 0x4809
#define HDIOC_FREESECTOR 0x480A // Returns the approximate amount of free space
#define HDIOC_IDLEIMM 0x480B

// 'h' command set
// Arbitrarily-named commands
#define HDIOC_GETTIME 0x6832
#define HDIOC_SETOSDMBR 0x6833 // arg = hddSetOsdMBR_t
#define HDIOC_GETSECTORERROR 0x6834
#define HDIOC_GETERRORPARTNAME 0x6835 // bufp = namebuffer[0x20]
#define HDIOC_READSECTOR 0x6836       // arg  = hddAtaTransfer_t
#define HDIOC_WRITESECTOR 0x6837      // arg  = hddAtaTransfer_t
#define HDIOC_SCEIDENTIFY 0x6838      // bufp = buffer for atadSceIdentifyDrive

// structs for DEVCTL commands

typedef struct
{
    u32 lba;
    u32 size;
    u8 data[0];
} hddAtaTransfer_t;

typedef struct
{
    u32 start;
    u32 size;
} hddSetOsdMBR_t;

//pfs

// IOCTL2 commands
// Command set 'p'
#define PIOCALLOC 0x7001
#define PIOCFREE 0x7002
#define PIOCATTRADD 0x7003
#define PIOCATTRDEL 0x7004
#define PIOCATTRLOOKUP 0x7005
#define PIOCATTRREAD 0x7006
#define PIOCINVINODE 0x7032 //Only available in OSD version. Arbitrarily named.

// DEVCTL commands
// Command set 'P'
#define PDIOC_ZONESZ 0x5001
#define PDIOC_ZONEFREE 0x5002
#define PDIOC_CLOSEALL 0x5003
#define PDIOC_GETFSCKSTAT 0x5004
#define PDIOC_CLRFSCKSTAT 0x5005

// Arbitrarily-named commands
#define PDIOC_SETUID 0x5032
#define PDIOC_SETGID 0x5033
#define PDIOC_SHOWBITMAP 0xFF

// I/O direction
#define PFS_IO_MODE_READ 0x00
#define PFS_IO_MODE_WRITE 0x01
