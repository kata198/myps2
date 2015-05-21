#!/bin/bash

echoAndExec() {
	echo $@;
	$@;
}

export TARGETS="bindir/myps2.exe \
	bindir/myps2_cmdonly.exe \
	bindir/myps2_quoted.exe \
	bindir/mypst2.exe \
	bindir/mypst2_cmdonly.exe \
	bindir/mypst2_quoted.exe \
	bindir/ps2.exe \
	bindir/ps2_cmdonly.exe \
	bindir/ps2_quoted.exe \
	bindir/pst2.exe \
	bindir/pst2_cmdonly.exe \
	bindir/pst2_quoted.exe"

make -f Makefile.cygwin all || ./build.sh;


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
