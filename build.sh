#!/bin/sh

[ "$1" ] || exit 1

export CC=${CC:-gcc}
export CFLAGS='-O2 -pipe -s -pedantic -std=c99 -fstack-protector-strong -fexceptions'
export LDFLAGS=-s
export PREFIX=${HOME}/.local

START_PWD="$PWD"
for name ; do
    cd $name
    if [ ! -d $name ] ; then
        git clone http://git.suckless.org/$name || exit 1
    else
        cd $name
        git clean -df
        git fetch --all
        git reset --hard
        cd - >/dev/null
    fi
    ls patches | while read -r patch ; do
	echo "===> applying $patch..."
        patch -l -p0 <patches/$patch || exit 1
    done
    cp -f config/config.h  $name/config.h  2>/dev/null ||:
    cp -f config/config.mk $name/config.mk 2>/dev/null ||:
    cd $name
    make clean >/dev/null 2>&1 ||:
    make CC=$CC
    make PREFIX="$PREFIX" install
    make clean >/dev/null 2>&1 ||:
    cd "$START_PWD"
done
