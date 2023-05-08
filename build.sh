#!/bin/sh -e

# -*-*-*-*-*-*-*-*- SETTINGS -*-*-*-*-*-*-*-*-*-*-*-*-*-*--*
DWM_VERSION=e81f17d4c196aaed6893fd4beed49991caa3e2a4      # 09 Apr 2023
# DWM_VERSION=bb2e7222baeec7776930354d0e9f210cc2aaad5f      # 08 Jul 2020
ST_VERSION=00085199039ee63acc7b1ecb7e3774a9c3552b2a       # 02 Oct 2022
DMENU_VERSION=1d2b462acf1210b8f86966b8dd9bb6e36e369ee1    # 23 Oct 2022
TABBED_VERSION=dabf6a25ab01107fc1e0464ee6a3e369d1626f97   # 12 May 2020
SURF_VERSION=d068a3878b6b9f2841a49cd7948cdf9d62b55585     # 08 Feb 2019
SENT_VERSION=2649e8d5334f7e37a1710c60fb740ecfe91b9f9e     # 13 May 2020
SLOCK_VERSION=35633d45672d14bd798c478c45d1a17064701aa9    # 25 Mar 2017

# do not remove this, needed for order of patches!
export LC_ALL=C

export PREFIX="${HOME}/.local"
# -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

usage() {
    die 'Usage: ./build.sh [dwm] [st] [dmenu] [tabbed] [surf]'
}

die() {
    >&2 printf '%s\n' "$*"
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

build() {
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
            slock) clone "$name" $sl $SLOCK_VERSION ;;
            *) usage
        esac

        cd "$name"

        if ls -A1q patches | grep -q . >/dev/null 2>&1 ; then
            for patch in patches/* ; do
                printf '%s' "===> applying ${patch#patches/}..."
                patch -s -l -p0 < "$patch" || exit 1
                printf '%s\n' ' OK!'
            done
        fi

        printf '%s\n\n' 'Patching succeeded!'

        [ -f cfg/config.h  ] && cp -f cfg/config.h  "$name"/config.h
        [ -f cfg/config.mk ] && cp -f cfg/config.mk "$name"/config.mk

        cd "$name"
        make clean
        make

        case $name in
            slock)
                >&2 printf '\n%s\n' 'Need to install to /usr/local due to suid:'
                case $(uname) in
                    Linux)
                        sudo make PREFIX=/usr/local install
                        sudo chmod u+s /usr/local/bin/slock
                        ;;
                    OpenBSD)
                        doas make PREFIX=/usr/local install
                        doas chmod u+s /usr/local/bin/slock
                esac

                # not all linux distros have nobody/nogroup
                if ! grep nobody /etc/passwd >/dev/null ; then
                    >&2 echo 'Non-priviledge user nobody does not exist, adding...'
                    sudo useradd nobody
                fi
                if ! grep nogroup /etc/group >/dev/null ; then
                    >&2 echo 'Non-priviledge group nobody does not exist, adding...'
                    sudo groupadd nogroup
                fi

                ;;
            *)
                make PREFIX="$PREFIX" install
        esac
    done
}

comptest() {
    printf 'Running compiler test...'
    printf '%s\n' 'int main() { return 0 ; }' > /tmp/$$.c
    ${CC:-gcc} /tmp/$$.c -o /tmp/$$
    chmod +x /tmp/$$
    if /tmp/$$ ; then
        rm /tmp/$$.c /tmp/$$
        printf ' %s\n\n' 'Passed!'
    else
        die "Something wrong with your compiler! Aborting."
    fi
}

main() {
    for cmd in make ${CC:-gcc} ld git patch ; do
        command -v "$cmd" >/dev/null || die "missing: $cmd"
    done
    comptest
    build "$@"
}

main "$@"
