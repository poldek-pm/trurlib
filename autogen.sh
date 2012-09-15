#!/bin/sh

FILE=n_array_new.c
PKG=trurlib
 
if [ ! -f $FILE ]; then
    cd `dirname $0`;
    if [ ! -f $FILE ]; then
	echo "`pwd`: not a $PKG directory"
	exit 1
    fi 	
fi

runcmd () {
    echo "Executing: $@"
    $@
    if [ $? -ne 0 ]; then
	echo "failed"
	exit 1
    fi
}

runcmd libtoolize --force --automake
runcmd aclocal
runcmd autoheader
runcmd autoconf
runcmd automake --add-missing --no-force

if [ -z "$1" -o "$1" != "--no-configure" ]; then
	CONFOPTS="--enable-maintainer-mode $@"
	runcmd ./configure $CONFOPTS
fi
