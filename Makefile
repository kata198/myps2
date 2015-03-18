#!/bin/bash -e

CFLAGS=-O3 -L/lib64
EXTRA_ALL_PROCS_FLAGS = $(shell pkg-config glib-2.0 --cflags --libs || echo '-DNO_GLIB')

all: bindir/myps2 bindir/myps2_cmdonly bindir/myps2_quoted bindir/ps2 bindir/ps2_cmdonly bindir/ps2_quoted

bindir/myps2: myps2.c
	gcc ${CFLAGS} myps2.c -o bindir/myps2

bindir/myps2_cmdonly: myps2.c
	gcc ${CFLAGS} -D CMD_ONLY myps2.c -o bindir/myps2_cmdonly

bindir/myps2_quoted: myps2.c
	gcc ${CFLAGS} -D QUOTE_ARGS myps2.c -o bindir/myps2_quoted

bindir/ps2: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS myps2.c -o bindir/ps2

bindir/ps2_cmdonly: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS myps2.c -o bindir/ps2_cmdonly

bindir/ps2_quoted: myps2.c
	gcc ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS myps2.c -o bindir/ps2_quoted

clean:
	rm bindir/*

install: all
	./install.sh
