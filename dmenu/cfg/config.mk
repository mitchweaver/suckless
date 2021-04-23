VERSION = 4.8

PREFIX = ${HOME}/.local
MANPREFIX = ${PREFIX}/share/man

# OpenBSD / Linux
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib
# NetBSD
# X11INC = /usr/X11R7/include
# X11LIB = /usr/X11R7/lib

XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

FREETYPELIBS = -lfontconfig -lXft

# LINUX
# FREETYPEINC = /usr/include/freetype2
# OPENBSD / NetBSD
FREETYPEINC = $(X11INC)/freetype2

INCS = -I${X11INC} -I${FREETYPEINC} -I${HOME}/.cache/themes

# OpenBSD / Linux
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lm
# NetBSD
# LIBS = -Wl,-R${X11LIB} -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -O2 -pipe -fstack-protector-strong -std=c99 -pedantic -Wall ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

CC = cc
