#!/bin/sh

if type xclip > /dev/null ; then
    xdg-open $(xclip -o) &
else
    xdg-open $(xsel -o) &
fi
