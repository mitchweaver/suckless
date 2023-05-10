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

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS} -O3 -pipe -fomit-frame-pointer -fipa-pta -fno-semantic-interposition -fno-trapping-math -fno-math-errno -fasynchronous-unwind-tables
CFLAGS = -std=c99 ${INCS} ${CPPFLAGS} -O3 -pipe -fomit-frame-pointer -fipa-pta -fno-semantic-interposition -fno-trapping-math -fno-math-errno -fasynchronous-unwind-tables  -O3 -fgraphite-identity -floop-nest-optimize -fdevirtualize-at-ltrans -fipa-pta -fno-semantic-interposition -flto=auto -fuse-linker-plugin -falign-functions=32
LDFLAGS  = ${LIBS} -lXrender -fuse-ld=mold -Wl,-O1 -Wl,--as-needed
