#!/usr/bin/make -f

CFLAGS=-O3 -L/lib64 
EXTRA_ALL_PROCS_FLAGS = $(shell pkg-config glib-2.0 --cflags --libs || echo '-DNO_GLIB')
$(shell mkdir -p bindir)

all: \
	bindir/myps2 \
	bindir/myps2_cmdonly \
	bindir/myps2_quoted \
	bindir/mypst2 \
	bindir/mypst2_cmdonly \
	bindir/mypst2_quoted \
	bindir/ps2 \
	bindir/ps2_cmdonly \
	bindir/ps2_quoted \
	bindir/pst2 \
	bindir/pst2_cmdonly \
	bindir/pst2_quoted 


bindir/myps2: myps2.c
	gcc ${CFLAGS} myps2.c -o bindir/myps2

bindir/myps2_cmdonly: myps2.c
	gcc ${CFLAGS} -D CMD_ONLY myps2.c -o bindir/myps2_cmdonly

bindir/myps2_quoted: myps2.c
	gcc ${CFLAGS} -D QUOTE_ARGS myps2.c -o bindir/myps2_quoted

bindir/mypst2: myps2.c
	gcc ${CFLAGS} myps2.c -D SHOW_THREADS -o bindir/mypst2

bindir/mypst2_cmdonly: myps2.c
	gcc ${CFLAGS} -D CMD_ONLY -D SHOW_THREADS myps2.c -o bindir/mypst2_cmdonly

bindir/mypst2_quoted: myps2.c
	gcc ${CFLAGS} -D QUOTE_ARGS -D SHOW_THREADS myps2.c -o bindir/mypst2_quoted

bindir/ps2: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS myps2.c -o bindir/ps2

bindir/ps2_cmdonly: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS myps2.c -o bindir/ps2_cmdonly

bindir/ps2_quoted: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS myps2.c -o bindir/ps2_quoted

bindir/pst2: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2

bindir/pst2_cmdonly: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_cmdonly

bindir/pst2_quoted: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_quoted

clean:
	rm -f bindir/*

install: all
	./install.sh
