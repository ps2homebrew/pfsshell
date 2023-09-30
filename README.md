# PFS (PlayStation File System) shell for POSIX-based systems

This tool allows you to browse and transfer files to and from PFS filesystems
using the command line.
This tool is useful for transferring configuration and media files used by
programs such as Open PS2 Loader and SMS.

## Quickstart

Binaries for Win32 are available here: <https://github.com/ps2homebrew/pfsshell/releases>
To start the program, provide the path to it on the command line:

    /path/to/pfsshell

You will get a prompt similar to the following:

    >

To get a list of commands, type in the following:

    > help

To select a device which can be a disk image or block device, type in the following:

    > device /path/to/device

On the Windows systems, the block devices can be used by using the UNC path. To get the UNC path, use this command:

```cmd
wmic diskdrive get Caption,DeviceID,InterfaceType
```

Once a device is selected, the prompt will change to the following:

    #

To mount a partition (for example, the `+OPL` partition), type in the following:

    # mount +OPL

The prompt will change to the following:

    +OPL:/#

To get a list of files or partitions, type in the following:

    +OPL:/# ls
    or
    +OPL:/# ls -l

To transfer a file from the current directory of the PFS partition to the current directory of the process, type in the following:

    +OPL:/# get example.txt

To transfer a file from the current directory of the process to the current directory of the PFS partition, type in the following:

    +OPL:/# put example.txt

Once you are finished looking around in the partition, type in the following:

    +OPL:/# umount

Once you are finished with the program, type in the following:

    # exit

## List of commands
    
Commands marked with DESTRUCTIVE could potentially wipe or remove important information. Remember to make backups if using these commands.

**Device Selection & Initialization**
    
    lcd [path] - print/change the local working directory
    
    device <device> - select the PS2 HDD device

    initialize - blank and create a partition on a PS2 HDD device (DESTRUCTIVE)

**Partition Editing**

    ls - list partitions (only when no partition mounted)

    mkpart <part_name> <size> - create a new partition (IMPORTANT size must be power of 2)

    mkfs <part_name> - blank and create PFS on a new partition (DESTRUCTIVE)

    mount <part_name> - mount a partition

    unmount - unmount the active partition

    rmpart <part_name> - remove a partition (DESTRUCTIVE)

**File & Folder Editing**
    pwd - print the current PS2 HDD directory

    ls  - list folders/files (only when partition mounted)

    cd <dir_name> - change directory

    mkdir <dir_name> - create a new directory

    rmdir <dir_name> - delete an existing directory

    pwd - print the current PS2 HDD directory

    get <file_name> - copy a file from PS2 HDD to the working directory

    put <file_name> - copy a file from the working directory to the PS2 HDD

    rm <file_name> - delete a file

    rename <curr_name> <new_name> - rename a file or directory


## pfsfuse

`pfsfuse` provides an ability to mount partition into host filesystem (like network folder).
`pfsfuse` supports physically connected drives, raw drive images as regular files, and an NBD network server from OPL.
Your system has to support fuse. For example, in the Linux `fuse` package should be installed, on the MacOS `macfuse` can be used.
The Unix users can use the following command for mounting the partition:

```sh
mkdir -p mountpoint
pfsfuse --partition=+OPL /path/to/device mountpoint/
df -h mountpoint/ # will show information about free space
```

`mountpoint` is accessible like a normal folder. Once you are finished looking around in the partition, type in the following:

    umount mountpoint

Note: for full access without root, use the argument `-o allow_other` when mounting.

### pfsfuse-win32 ###

Windows port uses [Dokan FUSE wrapper](https://github.com/dokan-dev/dokany) implementation as a base. Before using `pfsfuse` on Windows you should install Dokany. [Installation instructions](https://github.com/dokan-dev/dokany/wiki/Installation). After successful installation, you can use `pfsfuse` by pasting command in command line (CMD) or PowerShell with elevated privileges (run as administrator):
```sh
pfsfuse.exe --partition=+OPL \\.\PHYSICALDRIVE2 M -o volname=+OPL
or
pfsfuse.exe --partition=+OPL D:\ps2image.bin M -o volname=+OPL
```

Where `M` - is drive letter (please choose unused driver letter). `-o volname=+OPL` - will be volume name in File Explorer.
For unmounting, please locate dokanctl.exe and launch the following command in the elevated command prompt:
```sh
dokanctl.exe /u M
```
Where `M` - is mounted point drive letter.

## NBD server support

The latest Open PS2 Loader revisions have a built-in NBD server. `pfsshell/pfsfuse` have full support for an NBD block device, once an NBD server is mounted in the host filesystem.

## Building

This project can be built by using the Meson build system. For more information
about the system, please visit the following location: <https://mesonbuild.com/>

## Original project

The original project was created by Wizard of Oz. More information about the original project can be found at the following location:
<http://web.archive.org/web/20061220090822/http://hdldump.ps2-scene.org/>

## License

This project as a whole is licensed under the GNU General Public License GPL
version 2. Please read the `COPYING` file for more information.
The APA, PFS, and iomanX libraries are licensed under The Academic Free
License version 2. Please read the `COPYING.AFLv2` file for more information.
