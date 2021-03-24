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
	exec fallocate -l 1G test.img
	spawn ./build/pfsshell
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

proc run_cmd { cmd param arg expect } {

	set fmt1 "\[%d]\t%-16s"
	global count
	incr count
	puts -nonewline [format $fmt1 $count $cmd]
	send -- "$cmd $param\r"
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
			#exit 1
		}
		#eof {
		#	puts "eof"
		#}
	}
}

puts "Performing test :"

init_process

run_cmd "device" "test.img" "-re" "(.*)# "
run_cmd "initialize" "yes" "-exact" "# "
run_cmd "ls" "" "-exact" "
0x0001   128MB __mbr\r
0x0100   128MB __net\r
0x0100    16MB __system\r
0x0100    16MB __sysconf\r
0x0100    16MB __common\r
# "

run_cmd "mkfs" "__net" "-exact" "
pfs: Format: log.number = 8224, log.count = 16\r
pfs: Format sub: sub = 0, sector start = 8208, sector end = 8211\r
# "

run_cmd "mount" "__net" "-exact" "__net:/# "
run_cmd "pwd" "" "-exact" "\r
/\r
__net:/# "

run_cmd "mkdir" "directory" "-exact" "__net:/# "
run_cmd "cd" "directory" "-exact" "__net:/directory# "
run_cmd "put" "README.md" "-exact" "__net:/directory# "
run_cmd "rename" "README.md pfsshell.md" "-exact" "__net:/directory# "
run_cmd "exit" "" eof ""

# TODO: mkpart cannot be tested on file

#exec rm test.img
