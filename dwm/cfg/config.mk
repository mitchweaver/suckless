VERSION = 6.2

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# OpenBSD / Linux
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib
# NetBSD
#X11INC = /usr/X11R7/include
#X11LIB = /usr/X11R7/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft

# Linux
FREETYPEINC = /usr/include/freetype2
# OpenBSD / NetBSD
# FREETYPEINC = ${X11INC}/freetype2

INCS = -I${X11INC} -I${FREETYPEINC} -I${HOME}/.cache/themes

# OpenBSD / Linux
LIBS = -L${X11LIB} -lX11 -lXext ${XINERAMALIBS} ${FREETYPELIBS}
# NetBSD
#LIBS = -Wl,-R${X11LIB} -L${X11LIB} -lX11 -lXext ${XINERAMALIBS} ${FREETYPELIBS}

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -O2 -pipe -std=c99 -pedantic -Wall -Wno-deprecated-declarations ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS} -lXrender

CC = cc
