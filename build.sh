#!/bin/sh -e

# -*-*-*-*-*-*-*-*- SETTINGS -*-*-*-*-*-*-*-*-*-*-*-*-*-*--*
DWM_VERSION=f087d20e6e60a49c756936b4312f5d194d8e63b4      # 22 Apr 2020
ST_VERSION=33a9a456644ceb235ea6ce61282f3bdce7a8b547       # 11 Apr 2020
DMENU_VERSION=db6093f6ec1bb884f7540f2512935b5254750b30    # 03 Mar 2019
TABBED_VERSION=b5f9ec647aae2d9a1d3bd586eb7523a4e0a329a3   # 10 Mar 2018
SURF_VERSION=d068a3878b6b9f2841a49cd7948cdf9d62b55585     # 08 Feb 2019

export PREFIX=${HOME}/.local
export CFLAGS='-O2 -pipe -s -pedantic -std=c99 -fstack-protector-strong -fexceptions'
export LDFLAGS=-s
# -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

usage() {
    >&2 echo 'Usage: ./build.sh [dwm] [st] [dmenu] [tabbed] [surf]'
    exit 1
}

clone() {
    mkdir -p $1
    cd $1
    [ -d $1 ] || git clone $2/$1
    cd $1
    git clean -df
    git fetch --all
    git reset --hard $3
    cd "$START_PWD"
}

[ "$1" ] || usage

START_PWD=$(dirname "$0")
[ "$START_PWD" = . ] && START_PWD=$PWD
for name ; do
    cd "$START_PWD"

    sl=git://git.suckless.org
    case $name in
        st) clone $name $sl $ST_VERSION ;;
        dwm) clone $name $sl $DWM_VERSION ;;
        surf) clone $name $sl $SURF_VERSION ;;
        dmenu) clone $name $sl $DMENU_VERSION ;;
        tabbed) clone $name $sl $TABBED_VERSION ;;
        *) usage
    esac

    cd $name

    [ -d patches ] &&
    for patch in patches/* ; do
        printf '\n%s\n\n' "===> applying ${patch#patches/}..."
        patch -l -p0 <$patch
    done

    cp -f cfg/config.h  $name/config.h  2>/dev/null ||:
    cp -f cfg/config.mk $name/config.mk 2>/dev/null ||:

    cd $name
    make -s clean
    make -s -j${NPROC:-1} CC=${CC:-gcc}
    make -s PREFIX="$PREFIX" install
    make -s clean
done
