
Intent
======

To create very fast and specialized alternates to the standard ps command.

Each operation is a different command for maximum speed and simplicity.  All commands take an optional list of strings, filtering the results to commands containing those strings (much better than \| grep X \| grep Y ...)

It supports showing processes only, or threads and processes.
When showing threads, it draws a tree relating them to the parent process.

The output is VERY CLEAN and easilly parsed. For 99% of the usage of ps in the real-world, this displays all the info you need, quicker, simpler, and without looking up many flags.

It is also annoying to constantly have to type "| grep" over and over, and inefficent to feed to another program what one can do. Any additional arguments to these programs will filter based on the commandline string containing those items.


Performance
===========

Performance is amazing. On my personal machine, using the latest version, "myps" consistantly averaged 850% faster than "ps auxww | grep $USERNAME". None of the variant commands (\_whatever) have a noticable loss over the base. The "mypst2" and "pst2" commands (including threads) average 950% faster than the ps auxwwH equivlant.

Applications
============

There are  three major variants, and several minor variants.

The three majors are "myps2", "yourps2", and "ps2".

**ps2** - Shows processes by all users. All args provide search.

**yourps2** - Shows processes by another user. Second arg is username of other user, additional args provide search.

**myps2** - Shows processes by current user. All args provide search


ps2's output contains an additional second column of the username associated with the process.

ps2 and myps2 only show processes. To also show threads, use "pst2" or "mypst2", by themselves or with any of the below variants.

Both applications support representing the max argument length on the system.

myps2, ps2, yourps2 - All processes

mypst2, pst2, yourps2 - All processes and threads

myps2\_quoted, ps2\_quoted, yourps2\_quoted - These group each argument to the programs with quotes around them. These applications I find very useful with applications that have string arguments that may contain spaces (like a comment field).

myps2\_cmdonly, ps2\_cmdonly yourps2\_cmdonly - Show only the command name, no arguments. This can be useful for use with other applications that only use the command name, like killall and pidof.


**pidof2** - Prints pids of all matches against all users

**mypidof2** - Prints pids of all matches against current user

**yourpidof2** - Prints pids of all matches against a given user


Showing Real Paths of Executables
---------------------------------

Adding an "r" after the "2" of any command will resolve the full executable name (rather than argv[0]) and report that in the output instead. For example, instead of seeing "bash" you'd see "/usr/bin/bash" or wherever the actual executable lives. If permission is denied (like viewing the actual executable for a process other than your own, when you are not root), ps2 will fallback to the argv[0] (cmdline) reported value.

Examples: ps2r mypst2r ps2r\_cmdonly


Example
=======

	[media@silverslave myps2]$ yourpst2_quoted root
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

Filtering
=========

Add additional arguments to incorporate them as filters.

Examples:

  ps2 gcc myproject  # This will print any process by any user which contains "gcc" and "myproject" in its commandline string.

  myps2 firefox    # This will print all processes by current user which contain "firefox" in the commandline string.

  yourps2 root vim  # This will print all processes running as user "root" which contain "vim" in the commandline string.



Dependencies
============

Has no external dependencies than libc. Former glib-2.0 dependencies have been replaced by specific optimization functions.

INSTALLATION
============


Configuration
-------------

There is a provided "configure" script which can alter aspects of the program. You do not need to run this, for the most part it is for tweaking internals, for example to reduce memory usage at the cost of speed. 

There are a few display output options.

Run "configure --help" to see all options and a short description of them


Compile
-------

Run: make 


Install
-------

Run: make install


If DESTDIR environment variable is present, it will try to install to that directory. Otherwise, if /usr/bin is writeable, it will install there, else it will install in ~/bin


Cygwin
======

Cygwin comes with a VERY WEAK ps implementation. It will not even show arguments to programs! You can use this as an alternate, because it will.

LICENSE
=======

GPLv3 Copyright 2015 Tim Savannah <kata198@gmail.com>

