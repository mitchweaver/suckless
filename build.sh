#!/bin/sh -e

export CC=gcc
export CFLAGS='-O3 -pipe -s -w -fstack-protector-strong \
-fstack-clash-protection -fexceptions -Wl,-z,defs -Wl,-z,now \
-Wl,-z,relro -Wl,-z,noexecstack'
export LDFLAGS=-s
export PREFIX="${HOME}"/usr/local

[ "$1" ] || set -- dmenu sent st surf tabbed

START_PWD="$PWD"
for name in $@ ; do
    cd $name
    if [ ! -d $name ] ; then
        git clone https://git.suckless.org/$name
    else
        cd $name
        git clean -df
        git fetch --all
        git reset --hard
        cd - > /dev/null
    fi
    ls patches | while read -r patch ; do
		echo "* applying $patch..."
        patch -p0 --ignore-whitespace < patches/$patch
    done
    cp -f config/config.h $name/config.h 2>/dev/null ||:
    cp -f config/config.mk $name/config.mk 2>/dev/null ||:
    cd $name
    make
    make PREFIX="$PREFIX" install
    cd "$START_PWD"
done
