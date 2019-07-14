#!/bin/sh

COMMIT=6dc3978

download() {
    rm -rf surf 2> /dev/null
    git clone https://git.suckless.org/$name
    cd $name
    git reset --hard 6dc3978
    cd - > /dev/null
}

patchall() {
    for patch in $(ls patches) ; do
        patch -p0 < patches/$patch
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
        rm -f $junk 2> /dev/null
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

name=tabbed main
