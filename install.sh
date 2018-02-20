#!/bin/bash

echoAndExec() {
	echo $@;
	$@;
}

export TARGETS="bindir/myps2 \
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
	bindir/pst2r_quoted \
	bindir/pidof2 \
	bindir/pidoft2 \
	bindir/mypidof2 \
	bindir/mypidoft2 \
	bindir/yourpidof2 \
	bindir/yourpidoft2"

make all || ./build.sh;


if [ ! -z "$DESTDIR" ];
then
	echoAndExec cp -f ${TARGETS} $DESTDIR
elif [ -w "/usr/bin" ];
then
	echoAndExec cp -f ${TARGETS} /usr/bin/
else
	mkdir -p ~/bin
	echoAndExec cp -f ${TARGETS} ~/bin/
fi
