#!/usr/bin/env python3

import sys
from os import system

print(len(sys.argv))

arg = sys.argv[1]

# kill quote shenanigans for security
arg = arg.replace("'", "")
arg = arg.replace('"', "")
arg = arg.replace("`", "")
arg = arg.replace("$", "")

arg = "'" + arg + "'"

system('st -T surf-translate -n surf-translate -e /bin/sh -c "trans -b ' + arg + '| less -Q -R -s --tilde"')
