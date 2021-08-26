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

```sh
wmic diskdrive get Caption,DeviceID
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

## pfsfuse

`pfsfuse` provides an ability to mount partition into host filesystem (like network folder).
Your system has to support fuse. For example, on the WSL2 `fuse` package should be installed, on the MacOs `macfuse` package can be used.
The Unix users can use the following command for mounting the partition:

```sh
mkdir -p mountpoint
pfsfuse --partition=+OPL /path/to/device mountpoint/
df -h mountpoint/ # will show information about free space
```

`mountpoint` is accessible like a normal folder. Once you are finished looking around in the partition, type in the following:

    umount mountpoint

Note: for full access without root, use the argument `-o allow_other` when mounting.
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
