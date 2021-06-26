# PFS (PlayStation File System) shell for POSIX-based systems

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/7d9958516528483cad56ed4b4aab5654)](https://app.codacy.com/gh/ps2homebrew/pfsshell?utm_source=github.com&utm_medium=referral&utm_content=ps2homebrew/pfsshell&utm_campaign=Badge_Grade_Settings)

This tool allows you to browse and transfer files to and from PFS filesystems 
using the command line.  
This tool is useful for transfering configuration and media files used by 
programs such as Open PS2 Loader and SMS.  

## Quick start

Binaries for Win32 are availble here: https://github.com/uyjulian/pfsshell/releases  
To start the program, simply provide the path to it on the command line: 
```
/path/to/pfsshell
```
You will get a prompt similar to the following:
```
> 
```
To get a list of commands, type in the following:
```
> help
```
To select a device which can be a disk image or block device, type in the following:
```
> device /path/to/device
```
Block devices can be used on Windows by using the UNC path. To get the UNC path on Windows use this command: ```wmic diskdrive get Caption,DeviceID```.

Once a device is selected, the prompt will change to the following:
```
# 
```
To mount a partition (for example, the `+OPL` partition), type in the following:
```
# mount +OPL
```
The prompt will change to the following:
```
+OPL:/#
```
To get a list of files, type in the following:
```
+OPL:/# ls
```
To transfer a file from the current directory of the PFS partition to the current directory of the process, type in the following:  
```
+OPL:/# get example.txt
```
To transfer a file from the current directory of the process to the current directory of the PFS partition, type in the following:  
```
+OPL:/# put example.txt
```
Once you are finished looking around in the partition, type in the following:
```
+OPL:/# umount
```
Once you are finished with the program, type in the following:
```
# exit
```

## Building

This project can be built by using the Meson build system. For more information
about the system, please visit the following location: https://mesonbuild.com/

## Original project

The original project was created by Wizard of Oz. More information about the 
original project can be found at the following location:
http://web.archive.org/web/20061220090822/http://hdldump.ps2-scene.org/

## License

This project as a whole is licensed under the GNU General Public License GPL 
version 2. Please read the `COPYING` file for more information.  
The APA, PFS, and iomanX libraries are licensed under the The Academic Free 
License version 2. Please read the `COPYING.AFLv2` file for more infomation.  

