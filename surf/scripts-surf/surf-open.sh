#!/bin/sh
xidfile="/tmp/surf.xid"
uri=""

if [ "$#" -gt 0 ] ; then
	uri="$1"
fi

runtabbed() {
	tabbed -cd -r 2 surf -e '' "$uri" >"$xidfile"  2>/dev/null &
}

if [ ! -r "$xidfile" ] ; then
	runtabbed
else
	xid=$(cat "$xidfile")
	xprop -id "$xid" >/dev/null 2>&1
	if [ $? -gt 0 ] ; then
		runtabbed
	else
		surf -e "$xid" "$uri" >/dev/null 2>&1 &
	fi
fi

