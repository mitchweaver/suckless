#!/bin/sh

download() {
    rm -rf $name 2> /dev/null
    git clone http://git.suckless.org/$name
}

patchall() {
    for patch in $(ls patches) ; do
        patch --ignore-whitespace -p0 < patches/$patch
    done
}

build() {
    cd $name
    make
    # note: slock needs sudo for its setuid 
    sudo make install
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

name=slock main
