# sent version
VERSION = 1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# includes and libs
INCS = -I. -I/usr/include -I/usr/include/freetype2 -I${X11INC}
LIBS = -L/usr/lib -lc -lm -L${X11LIB} -lXft -lfontconfig -lX11
# OpenBSD (uncomment)
#INCS = -I. -I${X11INC} -I${X11INC}/freetype2
# FreeBSD (uncomment)
#INCS = -I. -I/usr/local/include -I/usr/local/include/freetype2 -I${X11INC}
#LIBS = -L/usr/local/lib -lc -lm -L${X11LIB} -lXft -lfontconfig -lX11

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_XOPEN_SOURCE=600
CFLAGS += -g -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += -g ${LIBS}
#CFLAGS += -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
#LDFLAGS += ${LIBS}

# compiler and linker
CC ?= cc
