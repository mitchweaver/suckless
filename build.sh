#!/bin/sh -e

# -*-*-*-*-*-*-*-*- SETTINGS -*-*-*-*-*-*-*-*-*-*-*-*-*-*--*
DWM_VERSION=cb3f58ad06993f7ef3a7d8f61468012e2b786cab      # 02 Feb 2019
ST_VERSION=51e19ea11dd42eefed1ca136ee3f6be975f618b1       # 18 Feb 2020
DMENU_VERSION=db6093f6ec1bb884f7540f2512935b5254750b30    # 03 Mar 2019
TABBED_VERSION=b5f9ec647aae2d9a1d3bd586eb7523a4e0a329a3   # 10 Mar 2018
SURF_VERSION=d068a3878b6b9f2841a49cd7948cdf9d62b55585     # 08 Feb 2019
SLSTATUS_VERSION=b14e039639ed28005fbb8bddeb5b5fa0c93475ac # 16 Feb 2019

export PREFIX=~/usr
export CFLAGS='-O2 -pipe -s -pedantic -std=c99 -fstack-protector-strong -fexceptions'
export LDFLAGS=-s
# -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

usage() {
    >&2 echo 'Usage: ./build.sh [dwm] [st] [dmenu] [tabbed] [surf] [slstatus]'
    exit 1
}

clone() {
    cd $1
    [ -d $1 ] || git clone git://git.suckless.org/$1
    cd $1
    git clean -df
    git fetch --all
    git reset --hard $2
    cd "$START_PWD"
}

[ "$1" ] || usage

START_PWD=$(dirname "$0")
[ "$START_PWD" = . ] && START_PWD=$PWD
for name ; do
    cd "$START_PWD"

    case $name in
        st) clone $name $ST_VERSION ;;
        dwm) clone $name $DWM_VERSION ;;
        surf) clone $name $SURF_VERSION ;;
        dmenu) clone $name $DMENU_VERSION ;;
        tabbed) clone $name $TABBED_VERSION ;;
        slstatus) clone $name $SLSTATUS_VERSION ;;
        *) usage
    esac

    cd $name

    ls patches | while read -r patch ; do
        printf '\n%s\n\n' "===> applying $patch..."
        patch -l -p0 <patches/$patch
    done

    cp -f cfg/config.h  $name/config.h  2>/dev/null ||:
    cp -f cfg/config.mk $name/config.mk 2>/dev/null ||:

    cd $name
    make -s clean
    make -s -j${NPROC:-1} CC=${CC:-gcc}
    make -s PREFIX="$PREFIX" install
    make -s clean
done
