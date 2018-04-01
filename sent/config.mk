# sent version
VERSION = 0.2

# paths
# PREFIX = /usr/local
PREFIX = ${HOME}/usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Linux
INCS = -I. -I/usr/include -I/usr/include/freetype2 -I${X11INC}
LIBS = -L/usr/lib -lc -lm -L${X11LIB} -lXft -lfontconfig -lX11 -lpng
# OpenBSD
# INCS = -I. -I/usr/include -I/usr/local/include -I/usr/X11R6/include -I/usr/X11R6/include/freetype2 -I${X11INC}
# LIBS = -L/usr/lib -L/usr/local/lib -lc -lm -L${X11LIB} -lXft -lfontconfig -lX11 -lpng

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_XOPEN_SOURCE=600

CFLAGS += -g -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS += -g ${LIBS}

CC=tcc
# CC = gcc -O3 -pipe
# CC = clang -O3 -pipe
