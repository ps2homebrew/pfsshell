#!/usr/bin/env expect

# Author : Bignaux Ronan
# KISS test with interactive programs

log_user 0
set timeout 5
set count 0

proc init_process {} {

	global spawn_id

	if { [file exists test.img] == 1} {
		file delete test.img
	}
	exec truncate -s 10G test.img

	spawn ../build/pfsshell
	match_max 100000
	expect -exact "pfsshell for POSIX systems\r
https://github.com/uyjulian/pfsshell\r
\r
This program uses pfs, apa, iomanX, \r
code from ps2sdk (https://github.com/ps2dev/ps2sdk)\r
\r
Type \"help\" for a list of commands.\r
\r
> "
}

proc run_cmd { name cmdparam arg expect } {


	send -- "$cmdparam\r"
	set fmt1 "\[%d]\t%-16s"
	global count
	incr count
	puts -nonewline [format $fmt1 $count $name]
	expect {
		$arg $expect {
			puts "\[\033\[01;32mpassed\033\[0m]"
		}
		timeout {
			puts "\[\033\[01;31mtimeout\033\[0m]"
			exit 1
		}
		-re "(.*)# " {
			puts "\[\033\[01;31mfailed\033\[0m]"
			# exit 1
		}
		#eof {
		#	puts "eof"
		#}
	}

}

puts "Performing test :"

init_process

run_cmd "device" "device test.img" "-re" "(.*)# "
# TODO: check more from device output
run_cmd "initialize" "initialize yes" "-exact" "
pfs: Format: log.number = 8224, log.count = 16\r
pfs: Format sub: sub = 0, sector start = 8208, sector end = 8211\r
pfs: Format: log.number = 8224, log.count = 16\r
pfs: Format sub: sub = 0, sector start = 8208, sector end = 8215\r
pfs: Format: log.number = 8224, log.count = 16\r
pfs: Format sub: sub = 0, sector start = 8208, sector end = 8223\r
pfs: Format: log.number = 8240, log.count = 16\r
pfs: Format sub: sub = 0, sector start = 8208, sector end = 8239\r
# "

run_cmd "ls partitions" "ls" "-exact" "
__mbr\r
__net/\r
__system/\r
__sysconf/\r
__common/\r
# "

run_cmd "mkpart" "mkpart PP.TEST 128M" "-exact" "# "
run_cmd "mkpart" "ls" "-exact" "
__mbr\r
__net/\r
__system/\r
__sysconf/\r
__common/\r
PP.TEST/\r
# "

run_cmd "mount" "mount __net" "-exact" "__net:/# "
run_cmd "pwd" "pwd" "-exact" "\r
/\r
__net:/# "

run_cmd "put" "put pfsshell_test.tcl" "-exact" "__net:/# "
run_cmd "rename" "rename pfsshell_test.tcl pfsshell.md" "-exact" "__net:/# "
run_cmd "mkdir" "mkdir directory" "-exact" "__net:/# "
run_cmd "ls files" "ls" "-exact" "
./\r
../\r
pfsshell.md\r
directory/\r
__net:/# "
run_cmd "cd" "cd directory" "-exact" "__net:/directory# "
run_cmd "cd" "cd .." "-exact" "__net:/# "
run_cmd "get" "get pfsshell.md" "-exact" "__net:/# "
run_cmd "rm" "rm pfsshell.md" "-exact" "__net:/# "
run_cmd "rmdir" "rmdir directory" "-exact" "__net:/# "
run_cmd "umount" "umount" "-exact" "# "
run_cmd "rmpart" "rmpart __net" "-exact" "# "
run_cmd "rmpart" "ls" "-exact" "
__mbr\r
__empty%\r
__system/\r
__sysconf/\r
__common/\r
PP.TEST/\r
# "

run_cmd "exit" "exit" eof ""
file delete pfsshell.md

#exec rm test.img
