#!/bin/bash -e

# GLIB support seems broken on cygwin so don't try to use it
CFLAGS="-O3 -L/lib64 -DNO_GLIB"

echoAndExec() {
	echo $@;
	$@;
}

echoAndExec gcc ${CFLAGS} myps2.c -o bindir/myps2
echoAndExec gcc ${CFLAGS} -D CMD_ONLY myps2.c -o bindir/myps2_cmdonly
echoAndExec gcc ${CFLAGS} -D QUOTE_ARGS myps2.c -o bindir/myps2_quoted
echoAndExec gcc ${CFLAGS} -D QUOTE_ARGS -D ALL_PROCS myps2.c -o bindir/ps2_quoted
echoAndExec gcc ${CFLAGS} -D ALL_PROCS myps2.c -o bindir/ps2
echoAndExec gcc ${CFLAGS} -D CMD_ONLY -D ALL_PROCS myps2.c -o bindir/ps2_cmdonly

echoAndExec gcc ${CFLAGS} -D SHOW_THREADS myps2.c -o bindir/mypst2
echoAndExec gcc ${CFLAGS} -D CMD_ONLY -D SHOW_THREADS myps2.c -o bindir/mypst2_cmdonly
echoAndExec gcc ${CFLAGS} -D QUOTE_ARGS -D SHOW_THREADS myps2.c -o bindir/mypst2_quoted
echoAndExec gcc ${CFLAGS} -D QUOTE_ARGS -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_quoted
echoAndExec gcc ${CFLAGS} -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2
echoAndExec gcc ${CFLAGS} -D CMD_ONLY -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_cmdonly
