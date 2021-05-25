VERSION = 1.4

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA
FREETYPELIBS = -lXft

# OpenBSD
# FREETYPEINC = /usr/X11R6/include/freetype2
# Linux
FREETYPEINC = /usr/include/freetype2

INCS = -I. -I/usr/include -I${X11INC} -I${FREETYPEINC}

CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS = -s ${LIBS}

# OpenBSD
# LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 -lXext -lXrandr ${XINERAMALIBS} ${FREETYPELIBS}
# Linux
LIBS = -L/usr/lib -lc -lcrypt -L${X11LIB} -lX11 -lXext -lXrandr ${XINERAMALIBS} ${FREETYPELIBS}

# Linux
CPPFLAGS = -DVERSION=\"${VERSION}\" -D_DEFAULT_SOURCE -DHAVE_SHADOW_H ${XINERAMAFLAGS}
# OpenBSD / FreeBSD
# CPPFLAGS = -DVERSION=\"${VERSION}\" -D_BSD_SOURCE ${XINERAMAFLAGS} -DHAVE_BSD_AUTH
# NetBSD
# CPPFLAGS = -DVERSION=\"${VERSION}\" -D_BSD_SOURCE -D_NETBSD_SOURCE ${XINERAMAFLAGS}

# OpenBSD
# COMPATSRC =
# Linux / FreeBSD / NetBSD
COMPATSRC = explicit_bzero.c

CC = cc
