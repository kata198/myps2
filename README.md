
Intent
======

To create very fast and specialized alternates to the standard ps command.

Each operation is a different command for maximum speed (direct codepath instead of conditional). No Arguments. Except myps, one optional argument of username to show.

It supports showing processes only, or threads and processes.
When showing threads, it draws a tree relating them to the parent process (ps cannot do --forest with -H).

The output is VERY CLEAN and easilly parsed. For 99% of the usage of ps in the real-world, this displays all the info you need, quicker, simpler, and without looking up many flags.


Performance
===========

Performance is amazing. On my personal machine, using the latest version, "myps" consistantly averaged 850% faster than "ps auxww | grep $USERNAME". None of the variant commands (\_whatever) have a noticable loss over the base. The "mypst2" and "pst2" commands (including threads) average 950% faster than the ps auxwwH equivlant.

Applications
============

There are two major variants, and several minor variants.

The two majors are "myps2" and "ps2".

*ps2* - Shows processes by all users
*myps2* - Shows processes by current user (with no args), or single arg for another username.

ps2's output contains an additional second column of the username associated with the process.

ps2 and myps2 only show processes. To also show threads, use "pst2" or "mypst2", by themselves or with any of the below variants.

Both applications support representing the max argument length on the system.

myps2, ps2 - All processes

mypst2, pst2 - All processes and threads

myps2\_quoted, ps2\_quoted - These group each argument to the programs with quotes around them. These applications I find very useful with applications that have string arguments that may contain spaces (like a comment field).

myps2\_cmdonly, ps2\_cmdonly - Show only the command name, no arguments. This can be useful for use with other applications that only use the command name, like killall and pidof.


Example
=======

	[media@silverslave myps2]$ mypst2\_quoted root
	1        /sbin/init
	6        /usr/lib/systemd/systemd-journald
	7        /usr/lib/systemd/systemd-udevd
	9        /usr/bin/bumblebeed
	1        /usr/lib/systemd/systemd-logind
	2        /usr/bin/irqbalance "--foreground"
	1        /usr/lib/xorg-server/Xorg "-nolisten" "tcp" ":0" "vt1" "-auth" "/tmp/serverauth.M0NhxCbWts"
	|
	\_____
		572        Thread [1] ( /usr/lib/xorg-server/Xorg )
	
	614        dhcpcd "-4" "-q" "-t" "30" "-L" "wlp13s0"
	690        /usr/lib/upower/upowerd
	|
	\____
		691        Thread [1] ( /usr/lib/upower/upowerd )
		692        Thread [2] ( /usr/lib/upower/upowerd )
	
	723        /usr/lib/udisks2/udisksd "--no-debug"
	|
	\_____
		724        Thread [1] ( /usr/lib/udisks2/udisksd )
		726        Thread [2] ( /usr/lib/udisks2/udisksd )
		730        Thread [3] ( /usr/lib/udisks2/udisksd )
		737        Thread [4] ( /usr/lib/udisks2/udisksd )



Dependencies
============

The only dependency is glib-2.0 for the ps2\* variants. If your system does not have glib-2.0 or does not have glib-2.0-devel package installed, you will get a warning and the code will be compiled using an alternate, much less efficient codepath.
With my testcases, the presence of glib-2.0 (used for hashmap impl) cuts runtime of ps2 by 2/3. (940 entries in 100ms vs 30ms)


INSTALLATION
============

Use make && make install

or
./build.sh
./install.sh


If DESTDIR environment variable is present, it will try to install to that directory. Otherwise, if /usr/bin is writeable, it will install there, else it will install in ~/bin

If you are building for cygwin, the glib-2.0 libs seem messed up (maybe just on my install). Uncomment the #define NO\_GLIB line at the top of myps2.c to work around this.


Cygwin
======

Cygwin comes with a VERY WEAK ps implementation. It will not even show arguments to programs! You can use this as an alternate, because it will.

LICENSE
=======

GPLv3 Copyright 2015 Tim Savannah <kata198@gmail.com>

