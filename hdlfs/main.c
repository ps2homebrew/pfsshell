#include <atad.h>
#include <errno.h>
#include <intrman.h>
#include <iomanX.h>
#include <irx.h>
#include <loadcore.h>
#include <thsemap.h>
#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <types.h>

#include <sys/stat.h>

#include "hdlfs.h"

#define MODNAME "hdl_filesystem_driver"
IRX_ID(MODNAME, 0x01, 0x01);

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINTF(args...) printf(args)
#else
#define DEBUG_PRINTF(args...)
#endif

/* APA IOCTL2 commands */
// structs for IOCTL2 commands
typedef struct
{
	unsigned int	sub;		// main(0)/subs(1+) to read/write
	unsigned int	sector;
	unsigned int	size;		// in sectors
	unsigned int	mode;		// ATAD_MODE_READ/ATAD_MODE_WRITE.....
	void	*buffer;
} hddIoctl2Transfer_t;

//
// IOCTL2 commands (From PS2FS)
//
#define APA_IOCTL2_NUMBER_OF_SUBS	0x00006803
#define APA_IOCTL2_TRANSFER_DATA	0x00006832	// used by pfs to read/write data :P
#define APA_IOCTL2_GETSIZE		0x00006833	// for main(0)/subs(1+)
#define APA_IOCTL2_GETSTART		0x00006836	// Get the sector number of the first sector of the partition.
#define APA_IOCTL2_FLUSH_CACHE		0x00006804

//
// DEVCTL commands (From PS2FS)
//
#define APA_DEVCTL_MAX_SECTORS		0x00004801	// max partition size(in sectors)

struct HDLFS_FileDescriptor{
	int	MountFD;
	int	SemaID;
	unsigned short int NumPartitions;
	unsigned short int CurrentPartNum;
	unsigned int	RelativeSectorNumber;	/* The current sector number, relative to the start of the current partition being accessed. */
	unsigned int	offset;			/* The current offset of the game in HDD sectors. */
	unsigned int	size;			/* The total size of all linked partitions in HDD sectors. */

	part_specs_t part_specs[65];
};

#define MAX_AVAILABLE_FDs		2

struct HDLFS_FileDescriptor FD_List[MAX_AVAILABLE_FDs];

static int unmount(unsigned int unit);

static int hdlfs_init(iop_device_t *fd){
	unsigned int i;
	iop_sema_t sema;

	memset(FD_List, 0, sizeof(FD_List));
	for(i=0; i<MAX_AVAILABLE_FDs; i++){
		FD_List[i].MountFD=-1;
		sema.attr=sema.option=0;
		sema.initial=sema.max=1;
		FD_List[i].SemaID=CreateSema(&sema);
	}

	return 0;
}

static int hdlfs_deinit(iop_device_t *fd){
	unsigned int i;

	for(i=0; i<MAX_AVAILABLE_FDs; i++){
		if(FD_List[i].MountFD>=0){
			unmount(i);
		}

		DeleteSema(FD_List[i].SemaID);
	}

	return 0;
}

static int hdlfs_format(iop_file_t *fd, const char *device, const char *blockdev, void *args, size_t arglen){
	int PartitionFD, result;
	unsigned int i, MaxSectorsPerPart, NumReservedSectors, NumberOfSectors, SectorNumber, SectorsInPart, SectorsRemaining;
	hdl_game_info HDLFilesystemData;

	MaxSectorsPerPart=devctl(blockdev, APA_DEVCTL_MAX_SECTORS, NULL, 0, NULL, 0);
	result=0;
	if((PartitionFD=open(blockdev, O_WRONLY, 0644))>=0){
		lseek(PartitionFD, HDL_GAME_DATA_OFFSET, SEEK_SET);

		memset(&HDLFilesystemData, 0, sizeof(hdl_game_info));
		HDLFilesystemData.magic=HDL_INFO_MAGIC;
		HDLFilesystemData.version=1;

		/* Parse arguments. */
		memcpy(HDLFilesystemData.gamename, ((struct HDLFS_FormatArgs*)args)->GameTitle, sizeof(HDLFilesystemData.gamename));
		HDLFilesystemData.hdl_compat_flags=0;
		HDLFilesystemData.ops2l_compat_flags=((struct HDLFS_FormatArgs*)args)->CompatFlags;
		HDLFilesystemData.dma_type=((struct HDLFS_FormatArgs*)args)->TRType;
		HDLFilesystemData.dma_mode=((struct HDLFS_FormatArgs*)args)->TRMode;
		strncpy(HDLFilesystemData.startup, ((struct HDLFS_FormatArgs*)args)->StartupPath, sizeof(HDLFilesystemData.startup));
		NumberOfSectors=((struct HDLFS_FormatArgs*)args)->NumSectors;
		HDLFilesystemData.layer1_start=((struct HDLFS_FormatArgs*)args)->Layer1Start;
		HDLFilesystemData.discType=((struct HDLFS_FormatArgs*)args)->DiscType;

		/* Set up partition data in the HDLoader game filesystem structure. */
		HDLFilesystemData.num_partitions=ioctl2(PartitionFD, APA_IOCTL2_NUMBER_OF_SUBS, NULL, 0, NULL, 0)+1;
		for(i=0, SectorNumber=0; i<HDLFilesystemData.num_partitions && result>=0; i++){
			if((result=ioctl2(PartitionFD, APA_IOCTL2_GETSTART, &i, sizeof(i), NULL, 0))>=0){
				NumReservedSectors=(i==0)?0x2000:4;	/* Main partitions have a 4MB reserved area, while sub-partitions have 2 reserved sectors.
										However, if a sub-partition has 2 reserved sectors, it means that there will be some leftover sectors at the end of the disk
										(The number of sectors will be indivisible by 4 - as 2048=512x4).

										Therefore, round up the number of reserved sectors for sub-partitions to 4.
									*/
				DEBUG_PRINTF("HDLFS format - part: %u, start: 0x%08lx\n", i, SectorNumber);
				HDLFilesystemData.part_specs[i].part_offset=SectorNumber;
				HDLFilesystemData.part_specs[i].data_start=result+NumReservedSectors;
				SectorsInPart=(ioctl2(PartitionFD, APA_IOCTL2_GETSIZE, &i, sizeof(i), NULL, 0)-NumReservedSectors)/4;
				SectorsRemaining=NumberOfSectors-SectorNumber;
				SectorsInPart=SectorsInPart>SectorsRemaining?SectorsRemaining:SectorsInPart;
				//Sanity check: Ensure that the partition size doesn't hit exactly 4GB or larger, or the size field will overflow.
				if(SectorsInPart>=0x200000){
					DEBUG_PRINTF("HDLFS: Error: Partition slice %d is too big for HDLFS.\n", );
					result=-EINVAL;
					break;
				}

				HDLFilesystemData.part_specs[i].part_size=SectorsInPart*2048;
				SectorNumber+=SectorsInPart;
			}
			else{
				DEBUG_PRINTF("HDLFS: Error: IOCTL2_GETSTART fail for partition number %u: %d\n", i, result);
			}
		}

		if(result>=0){
			/* Sanity check: Can the partition actually hold the number of sectors it should hold? */
			if(SectorNumber<NumberOfSectors){
				DEBUG_PRINTF("HDLFS - format failed: Partition is too small to contain game.\n");
				result=-ENOMEM;
			}
			else{
				result=(write(PartitionFD, &HDLFilesystemData, sizeof(hdl_game_info))!=sizeof(hdl_game_info))?-EIO:0;
			}
		}
		close(PartitionFD);
	}
	else result=PartitionFD;

	return result;
}

static int hdlfs_open(iop_file_t *fd, const char *path, int flags, int mode){
	int result;

	if(fd->unit<MAX_AVAILABLE_FDs && FD_List[fd->unit].MountFD>=0){
		fd->privdata=&FD_List[fd->unit];
		result=fd->unit;
	}
	else result=-ENODEV;

	return result;
}

static int hdlfs_close(iop_file_t *fd){
	return 0;
}

static int hdlfs_io_internal(iop_file_t *fd, void *buffer, int size, int mode){
	hddIoctl2Transfer_t CmdData;
	unsigned int SectorsToRead, SectorsRemaining, ReservedSectors, SectorsRemainingInPartition;
	int result;

	if((size%512!=0) || (buffer==NULL)){
		DEBUG_PRINTF("HDLFS: Invalid arguments for I/O! buffer: %p size: %d\n", buffer, size);
		return -EINVAL;
	}

	WaitSema(((struct HDLFS_FileDescriptor *)fd->privdata)->SemaID);

	SectorsRemaining=size/512;
	result=0;
	while(SectorsRemaining>0){
		/*
			What is done here:
				1. Determine which partition the data should be read/written from/to.
				2. Determine the number of sectors that can be read from the partition.
				3. Determine the first sector within the partition, at which data should be read/written from/to.
		*/

		SectorsRemainingInPartition=((struct HDLFS_FileDescriptor *)fd->privdata)->part_specs[((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum].part_size/512-((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber;
		/* If there are no more sectors in the current partition, move on. */
		if(SectorsRemainingInPartition<1){
			((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum++;
			((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber=0;
			SectorsRemainingInPartition=((struct HDLFS_FileDescriptor *)fd->privdata)->part_specs[((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum].part_size/512;
		}

		ReservedSectors=((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum==0?0x2000:4;
		SectorsToRead=SectorsRemaining>SectorsRemainingInPartition?SectorsRemainingInPartition:SectorsRemaining;

		/* If the calling program passes bogus arguments, the device driver called via ioctl2() should bail out. */
		CmdData.sub=((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum;
		CmdData.sector=ReservedSectors+((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber;
		CmdData.size=SectorsToRead;
		CmdData.mode=mode;
		CmdData.buffer=buffer;

		if((result=ioctl2(((struct HDLFS_FileDescriptor *)fd->privdata)->MountFD, APA_IOCTL2_TRANSFER_DATA, &CmdData, 0, NULL, 0))<0){
			DEBUG_PRINTF("HDLFS: I/O error occurred at partition %u, sector 0x%08lx, num sectors: %u, code: %d\n", ((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum, ((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber, SectorsToRead, result);
			break;
		}

		((struct HDLFS_FileDescriptor *)fd->privdata)->offset+=SectorsToRead;
		SectorsRemaining-=SectorsToRead;
		((unsigned char *)buffer)+=SectorsToRead*512;
		((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber+=SectorsToRead;
	}

	SignalSema(((struct HDLFS_FileDescriptor *)fd->privdata)->SemaID);

	return((result<0)?result:size);
}

static int hdlfs_read(iop_file_t *fd, void *buffer, int size){
	return((fd->mode&O_RDONLY)?hdlfs_io_internal(fd, buffer, size, ATA_DIR_READ):-EINVAL);
}

static int hdlfs_write(iop_file_t *fd, void *buffer, int size){
	return((fd->mode&O_WRONLY)?hdlfs_io_internal(fd, buffer, size, ATA_DIR_WRITE):-EROFS);
}

static long long int hdlfs_lseek_internal(iop_file_t *fd, long long int offset, int whence){
	unsigned int i;
	unsigned long long int PartOffsetInBytes;
	long long int result;

	if(fd->unit<MAX_AVAILABLE_FDs && FD_List[fd->unit].MountFD>=0){
		if(offset>=0 && offset%512==0){
			WaitSema(((struct HDLFS_FileDescriptor *)fd->privdata)->SemaID);

			result=-EINVAL;
			for(i=0; i<((struct HDLFS_FileDescriptor *)fd->privdata)->NumPartitions; i++){
				PartOffsetInBytes=(unsigned long long int)((struct HDLFS_FileDescriptor *)fd->privdata)->part_specs[i].part_offset*2048;
				if(PartOffsetInBytes<=offset && offset<PartOffsetInBytes+((struct HDLFS_FileDescriptor *)fd->privdata)->part_specs[i].part_size){
					switch(whence){
						case SEEK_SET:
							((struct HDLFS_FileDescriptor *)fd->privdata)->offset=offset/512;
							break;
						case SEEK_CUR:
							((struct HDLFS_FileDescriptor *)fd->privdata)->offset+=offset/512;
							break;
						case SEEK_END:
							((struct HDLFS_FileDescriptor *)fd->privdata)->offset=((struct HDLFS_FileDescriptor *)fd->privdata)->size-offset/512;
					}

					((struct HDLFS_FileDescriptor *)fd->privdata)->RelativeSectorNumber=((struct HDLFS_FileDescriptor *)fd->privdata)->offset-((struct HDLFS_FileDescriptor *)fd->privdata)->part_specs[i].part_offset*4;
					((struct HDLFS_FileDescriptor *)fd->privdata)->CurrentPartNum=i;
					result=((struct HDLFS_FileDescriptor *)fd->privdata)->offset*512;
					break;
				}
			}

			SignalSema(((struct HDLFS_FileDescriptor *)fd->privdata)->SemaID);
		}
		else result=-EINVAL;
	}
	else result=-ENODEV;

	return result;
}

static int hdlfs_lseek(iop_file_t *fd, unsigned int offset, int whence){
	return((int)hdlfs_lseek_internal(fd, offset, whence));
}

static int hdlfs_dopen(iop_file_t *fd, const char *path){
	return 0;
}

static int hdlfs_dclose(iop_file_t *fd){
	return 0;
}

static int hdlfs_getstat_filler(int unit, const char *path, iox_stat_t *stat, char *GameTitleOut, unsigned int MaxTitleLen, char *StartupPathOut, unsigned int MaxStartupPathLen){
	int PartFD, result;
	hdl_game_info HDLGameInfo;
	unsigned int i;
	u64 GameSize;

	if(unit<MAX_AVAILABLE_FDs && FD_List[unit].MountFD>=0){
		PartFD=FD_List[unit].MountFD;

		WaitSema(FD_List[unit].SemaID);

		lseek(PartFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
		if((result=read(PartFD, &HDLGameInfo, 1024))==1024){
			if(stat!=NULL){
				GameSize=0;
				for(i=0; i<HDLGameInfo.num_partitions; i++) GameSize+=HDLGameInfo.part_specs[i].part_size;

				memset(stat, 0, sizeof(iox_stat_t));
				stat->attr=((unsigned int)HDLGameInfo.dma_mode<<24)|((unsigned int)HDLGameInfo.dma_type<<16)|((unsigned int)HDLGameInfo.ops2l_compat_flags<<8)|HDLGameInfo.hdl_compat_flags;
				stat->size=(unsigned int)GameSize;
				stat->hisize=(unsigned int)(GameSize>>32);
				stat->private_0=(HDLGameInfo.discType<<16)|HDLGameInfo.num_partitions;
				stat->private_1=HDLGameInfo.layer1_start;
				stat->private_5=0x2000;	/* The relative LBA of the start of the game's disc image in the first partition. */
			}

			if(GameTitleOut!=NULL){
				memcpy(GameTitleOut, HDLGameInfo.gamename, (MaxTitleLen>sizeof(HDLGameInfo.gamename)?sizeof(HDLGameInfo.gamename):MaxTitleLen)-1);
				GameTitleOut[MaxTitleLen-1]='\0';
			}
			if(StartupPathOut!=NULL){
				strncpy(StartupPathOut, HDLGameInfo.startup, MaxStartupPathLen-1);
				StartupPathOut[MaxStartupPathLen-1]='\0';
			}

			result=0;
		}
		else result=-EIO;

		SignalSema(FD_List[unit].SemaID);
	}
	else result=-ENODEV;

	return result;
}

static int hdlfs_dread(iop_file_t *fd, iox_dirent_t *dirent){
	return 0;
}

static int hdlfs_getstat(iop_file_t *fd, const char *path, iox_stat_t *stat){
	return hdlfs_getstat_filler(fd->unit, path, stat, NULL, 0, NULL, 0);
}

static int hdlfs_chstat(iop_file_t *fd, const char *path, iox_stat_t *stat, unsigned int flags){
	int PartFD, result;
	hdl_game_info HDLGameInfo;

	if(fd->unit<MAX_AVAILABLE_FDs && FD_List[fd->unit].MountFD>=0){
		PartFD=FD_List[fd->unit].MountFD;

		WaitSema(FD_List[fd->unit].SemaID);

		lseek(PartFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
		if((result=read(PartFD, &HDLGameInfo, 1024))==1024){
			result=0;
			if(!(flags&(FIO_CST_MODE|FIO_CST_SIZE|FIO_CST_CT|FIO_CST_AT|FIO_CST_MT))){
				if(flags&FIO_CST_ATTR){
					HDLGameInfo.dma_mode=stat->attr>>24;
					HDLGameInfo.dma_type=(unsigned char)(stat->attr>>16);
					HDLGameInfo.ops2l_compat_flags=(unsigned char)(stat->attr>>8);
					HDLGameInfo.hdl_compat_flags=(unsigned char)stat->attr;
				}
				if(flags&FIO_CST_PRVT){
					HDLGameInfo.discType=stat->private_0>>16;
					HDLGameInfo.layer1_start=stat->private_1;
				}

				lseek(PartFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
				if((result=write(PartFD, &HDLGameInfo, 1024))!=1024){
					result=-EIO;
				}
			}
			else result=-EINVAL;	/* The contents of the mode, size and all time fields cannot be changed. */
		}
		else result=-EIO;

		SignalSema(FD_List[fd->unit].SemaID);
	}
	else result=-ENODEV;

	return result;
}

/* Here, mount the specified partition/device. */
static int hdlfs_mount(iop_file_t *fd, const char *mountpoint, const char *blockdev, int flags, void *arg, unsigned int arglen){
	iox_stat_t stat;
	hdl_game_info HDLGameInfo;
	int result;
	struct HDLFS_FileDescriptor *MountDescriptor;
	unsigned int part;

	if(fd->unit<MAX_AVAILABLE_FDs){
		if((result=getstat(blockdev, &stat))>=0){
			if(stat.mode==HDL_FS_MAGIC){
				MountDescriptor=&FD_List[fd->unit];

				WaitSema(MountDescriptor->SemaID);

				if((MountDescriptor->MountFD=open(blockdev, flags==FIO_MT_RDWR?O_RDWR:O_RDONLY, 0644))>=0){
					lseek(MountDescriptor->MountFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
					if((result=read(MountDescriptor->MountFD, &HDLGameInfo, sizeof(HDLGameInfo)))==sizeof(HDLGameInfo)){
						/* Calculate the size of all partitions combined. */
						MountDescriptor->size=0;
						MountDescriptor->NumPartitions=ioctl2(MountDescriptor->MountFD, APA_IOCTL2_NUMBER_OF_SUBS, NULL, 0, NULL, 0)+1;
						for(part=0; part<MountDescriptor->NumPartitions; part++) MountDescriptor->size+=ioctl2(MountDescriptor->MountFD, APA_IOCTL2_GETSIZE, &part, sizeof(part), NULL, 0);

						MountDescriptor->offset=MountDescriptor->CurrentPartNum=MountDescriptor->RelativeSectorNumber=0;
						memcpy(MountDescriptor->part_specs, HDLGameInfo.part_specs, sizeof(MountDescriptor->part_specs));
						result=fd->unit;
					}
				}
				else{
					result=MountDescriptor->MountFD;
					DEBUG_PRINTF("Failed to open partition: %s, result: %d\n", path, result);
				}

				SignalSema(MountDescriptor->SemaID);
			}
			else result=-EMFILE;
		}
	}
	else result=-ENODEV;

	return result;
}

static int unmount(unsigned int unit){
	int result;

	if(unit<MAX_AVAILABLE_FDs && FD_List[unit].MountFD>=0){
		WaitSema(FD_List[unit].SemaID);

		ioctl2(FD_List[unit].MountFD, APA_IOCTL2_FLUSH_CACHE, NULL, 0, NULL, 0);
		close(FD_List[unit].MountFD);
		FD_List[unit].MountFD=-1;

		SignalSema(FD_List[ unit].SemaID);

		result=0;
	}
	else result=-ENODEV;

	return result;
}

static int hdlfs_umount(iop_file_t *fd, const char *mountpoint){
	return unmount(fd->unit);
}

static long long hdlfs_lseek64(iop_file_t *fd, long long int offset, int whence){
	return(hdlfs_lseek_internal(fd, offset, whence));
}

static inline int hdlfs_UpdateGameTitle(int unit, const char *NewName, unsigned int len){
	int result, PartFD;
	hdl_game_info HDLGameInfo;

	if(len<sizeof(HDLGameInfo.gamename)){
		PartFD=FD_List[unit].MountFD;

		lseek(PartFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
		if((result=read(PartFD, &HDLGameInfo, sizeof(hdl_game_info)))==sizeof(hdl_game_info)){
			memcpy(HDLGameInfo.gamename, NewName, len);
			memset(&HDLGameInfo.gamename[len], 0, sizeof(HDLGameInfo.gamename)-len);
			lseek(PartFD, HDL_GAME_DATA_OFFSET, SEEK_SET);
			result=(write(PartFD, &HDLGameInfo, sizeof(hdl_game_info))!=sizeof(hdl_game_info))?-EIO:0;
		}
		else result=-EIO;
	}
	else result=-EINVAL;

	return result;
}

static int hdlfs_devctl(iop_file_t *fd, const char *path, int cmd, void *arg, unsigned int arglen, void *buf, unsigned int buflen){
	int result;

	if(fd->unit<MAX_AVAILABLE_FDs && FD_List[fd->unit].MountFD>=0){
		switch(cmd){
			case HDLFS_DEVCTL_GET_STARTUP_PATH:
				result=hdlfs_getstat_filler(fd->unit, path, NULL, NULL, 0, buf, buflen);
				break;
			case HDLFS_DEVCTL_GET_TITLE:
				result=hdlfs_getstat_filler(fd->unit, path, NULL, buf, buflen, NULL, 0);
				break;
			case HDLFS_DEVCTL_SET_TITLE:
				result=hdlfs_UpdateGameTitle(fd->unit, arg, arglen);
				break;
			default:
				result=-EINVAL;
		}
	}
	else result=-ENODEV;

	return result;
}

static int hdlfs_NulldevFunction(void){
	return -EIO;
}

/* Device driver I/O functions */
static iop_device_ops_t hdlfs_functarray={
	&hdlfs_init,			/* INIT */
	&hdlfs_deinit,			/* DEINIT */
	&hdlfs_format,			/* FORMAT */
	&hdlfs_open,			/* OPEN */
	&hdlfs_close,			/* CLOSE */
	&hdlfs_read,			/* READ */
	&hdlfs_write,			/* WRITE */
	&hdlfs_lseek,			/* LSEEK */
	(void*)&hdlfs_NulldevFunction,	/* IOCTL */
	(void*)&hdlfs_NulldevFunction,	/* REMOVE */
	(void*)&hdlfs_NulldevFunction,	/* MKDIR */
	(void*)&hdlfs_NulldevFunction,	/* RMDIR */
	&hdlfs_dopen,			/* DOPEN */
	&hdlfs_dclose,			/* DCLOSE */
	&hdlfs_dread,			/* DREAD */
	&hdlfs_getstat,			/* GETSTAT */
	&hdlfs_chstat,			/* CHSTAT */
	(void*)&hdlfs_NulldevFunction,	/* RENAME */
	(void*)&hdlfs_NulldevFunction,	/* CHDIR */
	(void*)&hdlfs_NulldevFunction,	/* SYNC */
	&hdlfs_mount,			/* MOUNT */
	&hdlfs_umount,			/* UMOUNT */
	&hdlfs_lseek64,			/* LSEEK64 */
	&hdlfs_devctl,			/* DEVCTL */
	(void*)&hdlfs_NulldevFunction,	/* SYMLINK */
	(void*)&hdlfs_NulldevFunction,	/* READLINK */
	(void*)&hdlfs_NulldevFunction	/* IOCTL2 */
};

static const char hdlfs_dev_name[]="hdl";

static iop_device_t hdlfs_dev={
	hdlfs_dev_name,			/* Device name */
	IOP_DT_FS|IOP_DT_FSEXT,		/* Device type flag */
	1,				/* Version */
	"HDLoader filesystem driver",	/* Description. */
	&hdlfs_functarray		/* Device driver function pointer array */
};

/* Entry point */
int _start(int argc, char **argv)
{
	DelDrv(hdlfs_dev_name);
	AddDrv(&hdlfs_dev);

	return MODULE_RESIDENT_END;
}
