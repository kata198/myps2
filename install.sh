#!/bin/bash

echoAndExec() {
	echo $@;
	$@;
}

TARGETS="bindir/myps2 bindir/myps2_cmdonly bindir/myps2_quoted bindir/ps2 bindir/ps2_cmdonly bindir/ps2_quoted"
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
