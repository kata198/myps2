#!/usr/bin/make -f

CFLAGS ?= -O3 -mtune=native -Wall
EXTRA_ALL_PROCS_FLAGS=
CC ?= gcc

$(shell mkdir -p bindir)

all: \
	bindir/myps2 \
	bindir/myps2_cmdonly \
	bindir/myps2_quoted \
	bindir/mypst2 \
	bindir/mypst2_cmdonly \
	bindir/mypst2_quoted \
	bindir/myps2r \
	bindir/myps2r_cmdonly \
	bindir/myps2r_quoted \
	bindir/mypst2r \
	bindir/mypst2r_cmdonly \
	bindir/mypst2r_quoted \
	bindir/yourps2 \
	bindir/yourps2_cmdonly \
	bindir/yourps2_quoted \
	bindir/yourpst2 \
	bindir/yourpst2_cmdonly \
	bindir/yourpst2_quoted \
	bindir/yourps2r \
	bindir/yourps2r_cmdonly \
	bindir/yourps2r_quoted \
	bindir/yourpst2r \
	bindir/yourpst2r_cmdonly \
	bindir/yourpst2r_quoted \
	bindir/ps2 \
	bindir/ps2_cmdonly \
	bindir/ps2_quoted \
	bindir/pst2 \
	bindir/pst2_cmdonly \
	bindir/pst2_quoted \
	bindir/ps2r \
	bindir/ps2r_cmdonly \
	bindir/ps2r_quoted \
	bindir/pst2r \
	bindir/pst2r_cmdonly \
	bindir/pst2r_quoted 

DEP_FILES=myps2.c myps2_config.h

bindir/myps2: ${DEP_FILES}
	${CC} ${CFLAGS} myps2.c -o bindir/myps2

bindir/myps2r: ${DEP_FILES}
	${CC} ${CFLAGS} myps2.c -D REPLACE_EXE_NAME -o bindir/myps2r

bindir/myps2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D CMD_ONLY myps2.c -o bindir/myps2_cmdonly

bindir/myps2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D CMD_ONLY myps2.c -D REPLACE_EXE_NAME -o bindir/myps2r_cmdonly

bindir/myps2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D QUOTE_ARGS myps2.c -o bindir/myps2_quoted

bindir/myps2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D QUOTE_ARGS myps2.c -D REPLACE_EXE_NAME -o bindir/myps2r_quoted

bindir/mypst2: ${DEP_FILES}
	${CC} ${CFLAGS} myps2.c -D SHOW_THREADS -o bindir/mypst2

bindir/mypst2r: ${DEP_FILES}
	${CC} ${CFLAGS} myps2.c -D SHOW_THREADS -D REPLACE_EXE_NAME -o bindir/mypst2r

bindir/mypst2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D CMD_ONLY -D SHOW_THREADS myps2.c -o bindir/mypst2_cmdonly

bindir/mypst2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D CMD_ONLY -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/mypst2r_cmdonly

bindir/mypst2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D QUOTE_ARGS -D SHOW_THREADS myps2.c -o bindir/mypst2_quoted

bindir/mypst2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D QUOTE_ARGS -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/mypst2r_quoted


bindir/yourps2: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS myps2.c -o bindir/yourps2

bindir/yourps2r: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D REPLACE_EXE_NAME myps2.c -o bindir/yourps2r

bindir/yourps2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D CMD_ONLY myps2.c -o bindir/yourps2_cmdonly

bindir/yourps2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D CMD_ONLY -D REPLACE_EXE_NAME myps2.c -o bindir/yourps2r_cmdonly

bindir/yourps2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D QUOTE_ARGS myps2.c -o bindir/yourps2_quoted

bindir/yourps2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D QUOTE_ARGS -D REPLACE_EXE_NAME myps2.c -o bindir/yourps2r_quoted

bindir/yourpst2: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS myps2.c -D SHOW_THREADS -o bindir/yourpst2

bindir/yourpst2r: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS myps2.c -D SHOW_THREADS -D REPLACE_EXE_NAME -o bindir/yourpst2r

bindir/yourpst2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D CMD_ONLY -D SHOW_THREADS myps2.c -o bindir/yourpst2_cmdonly

bindir/yourpst2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D CMD_ONLY -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/yourpst2r_cmdonly

bindir/yourpst2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D QUOTE_ARGS -D SHOW_THREADS myps2.c -o bindir/yourpst2_quoted

bindir/yourpst2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} -D OTHER_USER_PROCS -D QUOTE_ARGS -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/yourpst2r_quoted


bindir/ps2: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS myps2.c -o bindir/ps2

bindir/ps2r: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS -D REPLACE_EXE_NAME myps2.c -o bindir/ps2r

bindir/ps2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS myps2.c -o bindir/ps2_cmdonly

bindir/ps2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS -D REPLACE_EXE_NAME myps2.c -o bindir/ps2r_cmdonly

bindir/ps2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS myps2.c -o bindir/ps2_quoted

bindir/ps2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS -D REPLACE_EXE_NAME myps2.c -o bindir/ps2r_quoted

bindir/pst2: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2

bindir/pst2r: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D ALL_PROCS -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/pst2r

bindir/pst2_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_cmdonly

bindir/pst2r_cmdonly: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D CMD_ONLY -D ALL_PROCS -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/pst2r_cmdonly

bindir/pst2_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS -D SHOW_THREADS myps2.c -o bindir/pst2_quoted

bindir/pst2r_quoted: ${DEP_FILES}
	${CC} ${CFLAGS} ${EXTRA_ALL_PROCS_FLAGS} -D QUOTE_ARGS -D ALL_PROCS -D SHOW_THREADS -D REPLACE_EXE_NAME myps2.c -o bindir/pst2r_quoted


clean:
	rm -f bindir/*

distclean:
	rm -f bindir/*
	echo " /* myps2_config.h - Run ./configure to generate and change defaults. */ " > myps2_config.h

install: all
	./install.sh
