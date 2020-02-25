#!/bin/sh -e

usage() {
    >&2 echo 'Usage: ./build.sh [dwm] [st] [dmenu] [tabbed]'
    exit 1
}

[ "$1" ] || usage

export CC=${CC:-gcc}
export CFLAGS='-O2 -pipe -s -pedantic -std=c99 -fstack-protector-strong -fexceptions'
export LDFLAGS=-s
export PREFIX=${HOME}/.local

ST_VERSION=0.8.2
DWM_VERSION=6.2
DMENU_VERSION=4.9
TABBED_VERSION=0.6

START_PWD="$PWD"
for name ; do
    cd $name

    case $name in
        dwm) url=https://dl.suckless.org/dwm/dwm-$DWM_VERSION.tar.gz ;;
        st) url=https://dl.suckless.org/st/st-$ST_VERSION.tar.gz ;;
        dmenu) url=https://dl.suckless.org/tools/dmenu-$DMENU_VERSION.tar.gz ;;
        tabbed) url=https://dl.suckless.org/tools/tabbed-$TABBED_VERSION.tar.gz ;;
        *) usage
    esac

    file="$(basename "$url")"
    curl -q -L -C - -# --url "$url" --output "$file"
    mkdir -p dl
    tar -xzf "$file" -C dl
    [ -d $name ] && rm -rf $name
    mv dl/$name-* $name
    rmdir dl

    ls patches | while read -r patch ; do
        printf '\n%s\n\n' "===> applying $patch..."
        patch -l -p0 <patches/$patch
    done

    cp -f config/config.h  $name/config.h  2>/dev/null
    cp -f config/config.mk $name/config.mk 2>/dev/null

    cd $name
    make CC="$CC"
    make PREFIX="$PREFIX" install
    cd "$START_PWD"
done
