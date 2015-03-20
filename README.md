
Intent
======

To create very fast and specialized alternates to the standard ps command.

Each operation is a different command for maximum speed (direct codepath instead of conditional). No Arguments.

Cygwin comes with a VERY WEAK ps implementation. It will not even show arguments to programs! You can use this as an alternate, because it will.

Applications
============


There are two variants of the three major applications.

The "myps2" variants only show processes running by the current user. The "ps2" variants show all users and has additional column listing owning user's name..

Both applications support representing the max argument length on the system.


myps2, ps2 - All processes

myps2_quotes, ps2_quotes - These group each argument to the programs with quotes around them. These applications I find very useful with applications that have string arguments that may contain spaces (like a comment field).

myps2_cmdonly, ps2_cmdonly - Show only the command name, no arguments. This can be useful for use with other applications that only use the command name, like killall and pidof.


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

If you are building for cygwin, the glib-2.0 libs seem messed up (maybe just on my install). Uncomment the #define NO_GLIB line at the top of myps2.c to work around this.

LICENSE
=======

GPLv3 Copyright 2015 Tim Savannah <kata198@gmail.com>

