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
bkm="${HOME}/var/files/bkm"
tmp=/tmp/bmk.tmp

st -n floating-st -T floating-st -e \
    ranger --cmd='set viewmode!' --choosefiles=$tmp "$bkm"

xprop -id $xid -f _SURF_GO 8s -set _SURF_GO "$(cat "$(cat $tmp)")"
rm -f $tmp
