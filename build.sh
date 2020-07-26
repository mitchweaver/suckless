#!/bin/sh -e

# -*-*-*-*-*-*-*-*- SETTINGS -*-*-*-*-*-*-*-*-*-*-*-*-*-*--*
DWM_VERSION=bb2e7222baeec7776930354d0e9f210cc2aaad5f      # JUL 8 2020
ST_VERSION=fa253f077f19b3220c7655b81bd91e52f4367803       # 19 Jun 2020
DMENU_VERSION=9b38fda6feda68f95754d5b8932b1a69471df960    # 11 Jun 2020
TABBED_VERSION=dabf6a25ab01107fc1e0464ee6a3e369d1626f97   # 12 May 2020
SURF_VERSION=d068a3878b6b9f2841a49cd7948cdf9d62b55585     # 08 Feb 2019
SENT_VERSION=2649e8d5334f7e37a1710c60fb740ecfe91b9f9e     # 13 May 2020

export PREFIX=${HOME}/.local
export CFLAGS='-O2 -pipe -s -pedantic -std=c99 -fstack-protector-strong -fexceptions'
export LDFLAGS=-s
# -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

usage() {
    >&2 echo 'Usage: ./build.sh [dwm] [st] [dmenu] [tabbed] [surf]'
    exit 1
}

clone() {
    mkdir -p "$1"
    cd "$1"
    [ -d "$1" ] || git clone "$2/$1"
    cd "$1"
    git clean -df
    git fetch --all
    git reset --hard "$3"
    cd "$START_PWD"
}

[ "$1" ] || usage

START_PWD=$(dirname "$0")
[ "$START_PWD" = . ] && START_PWD=$PWD
for name ; do
    cd "$START_PWD"

    sl=git://git.suckless.org
    case $name in
        st) clone "$name" $sl $ST_VERSION ;;
        dwm) clone "$name" $sl $DWM_VERSION ;;
        surf) clone "$name" $sl $SURF_VERSION ;;
        dmenu) clone "$name" $sl $DMENU_VERSION ;;
        tabbed) clone "$name" $sl $TABBED_VERSION ;;
        sent) clone "$name" $sl $SENT_VERSION ;;
        *) usage
    esac

    cd "$name"

    [ -d patches ] &&
    for patch in patches/* ; do
        printf '\n%s\n\n' "===> applying ${patch#patches/}..."
        patch -l -p0 < "$patch" || exit 1
    done

    cp -f cfg/config.h  "$name"/config.h  2>/dev/null ||:
    cp -f cfg/config.mk "$name"/config.mk 2>/dev/null ||:

    cd "$name"
    make -s clean
    make -s -j"${NPROC:-1}" CC="${CC:-gcc}"
    make -s PREFIX="$PREFIX" install
    make -s clean
done
