How this driver is designed to be used for creating game partitions:
--------------------------------------------------------------------

1. A partition is created with the accompanying modified PS2HDD module. If the partition is not large enough to hold the entire game, create sufficient sub-partitions to store the entire game.
	Note: The size of the sub-partitions is expected to be of the same size as the main partition, with the exception of the last sub partition. All partitions are expected to be of the largest allowable size, except for the last partition (Which may be smaller).
2. mount the created partition with HDLFS.
	HDLFS shall allocate a handle for future I/O access to the partition.
3. Format the partition, specifying the game's data.
	HDLFS shall initialize the HDL Game information area (Sector offset 0x800 in the partition extended attribute area) with the specified parameters.
4. Open the mount point and begin writing to the partition.
	HDLFS will automatically determine the partition to write data to.
5. Close the mount point.
6. Unmount the mount point. Failing to do so might result in data loss.
	HDLFS might update game installation data and will close the mounted volume at this point.