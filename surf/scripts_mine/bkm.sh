#!/bin/sh
#
# http://github.com/mitchweaver/suckless
#
# a bookmarking system with folder support
#
# idea: folder of folders, containing bookmarks
#       a bookmark is a textfile containing its link,
#       named whatever you wish to be its label
#

xid="$1"
dir="${HOME}/var/files/bkm"

go() { xprop -id $xid -f _SURF_GO 8s -set _SURF_GO "$1" ; }

st -n floating-st -T floating-st \
    -e ranger --choosefiles /tmp/bmk.tmp "$dir"

go "$(cat "$(cat /tmp/bmk.tmp)")"

rm -f /tmp/bmk.tmp
