VERSION = 2.0

PREFIX = ${HOME}/usr/local
MANPREFIX = ${PREFIX}/share/man
LIBPREFIX = ${PREFIX}/lib/surf

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

GTKINC = `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0`
GTKLIB = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0`

INCS = -I. -I/usr/include -I${X11INC} ${GTKINC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${GTKLIB} -lgthread-2.0

CPPFLAGS = -DVERSION=\"${VERSION}\" -DWEBEXTDIR=\"${LIBPREFIX}\" -D_DEFAULT_SOURCE
CFLAGS = -O2 -fstack-protector-all -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

CC = cc
