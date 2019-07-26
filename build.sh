#!/bin/sh -e

download() {
    if [ ! -d $name ] ; then
        git clone https://git.suckless.org/$name
    else
        cd $name
        git clean -df
        git fetch --all
        git reset --hard
        cd - > /dev/null
    fi
}

patchall() {
    ls patches | while read -r patch ; do
        patch -p0 --ignore-whitespace < patches/$patch
    done
}

build() {
    cd $name
    make
    make install
    cd - > /dev/null
}

clean() {
    cd $name
    for junk in FAQ.md TODO.md README '*.orig' '*.rej' ; do
        rm -f $junk 2> /dev/null ||:
    done
    make clean
    cd - > /dev/null
}

main() {
    download
    patchall
    cp -f config/config.h $name/config.h
    cp -f config/config.mk $name/config.mk
    build
    clean > /dev/null
}

[ "$1" = all ] && set -- dmenu sent st surf tabbed

# fetch/build all in parallel
for arg in "$@" ; do
    {
        startdir="$PWD"
        cd "$arg"
        name=$arg main
        cd "$startdir"
    } &
done
wait
