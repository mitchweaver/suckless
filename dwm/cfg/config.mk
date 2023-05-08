VERSION = 6.2

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

FREETYPELIBS = -lfontconfig -lXft

FREETYPEINC = /usr/include/freetype2

INCS = -I${X11INC} -I${FREETYPEINC} -I${HOME}/.cache/themes

LIBS = -L${X11LIB} -lX11 -lXext ${XINERAMALIBS} ${FREETYPELIBS}

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -w -O3 -pipe -std=c99 ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS} -lXrender -fuse-ld=mold -Wl,-O1 -Wl,--as-needed
