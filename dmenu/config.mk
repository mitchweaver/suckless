VERSION = 4.8

PREFIX = ${HOME}/usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

FREETYPELIBS = -lfontconfig -lXft

# LINUX 
FREETYPEINC = /usr/include/freetype2
# OpenBSD
#FREETYPEINC = ${X11INC}/freetype2

INCS = -I${X11INC} -I${FREETYPEINC}
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -lm

CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
CFLAGS   = -std=c99 -pedantic -Wall -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = -s ${LIBS}

#CC = tcc -w
CC = gcc -O3 -pipe -std=c99 -fstack-protector-all -fpie
#CC = clang -O3 -pipe -std=c99 -fstack-protector-all -fpie -Wl,-pie -Wl,-z,relro
