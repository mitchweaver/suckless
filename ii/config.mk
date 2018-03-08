# Customize to fit your system
VERSION = 1.8

# paths
PREFIX   = /usr/local
BINDIR   = ${PREFIX}/bin
MANDIR   = ${PREFIX}/share/man
MAN1DIR  = ${MANDIR}/man1
DOCDIR   = ${PREFIX}/share/doc/ii

# includes and libs
INCLUDES = -I. -I/usr/include
LIBS     =

# compiler
CC       = cc

# debug
#CFLAGS  = -g -O0 -pedantic -Wall ${INCLUDES} -DVERSION=\"${VERSION}\" \
#	-std=c99 -D_DEFAULT_SOURCE
#LDFLAGS = ${LIBS}

# release
CFLAGS   = -Os ${INCLUDES} -DVERSION=\"${VERSION}\" -std=c99 -D_DEFAULT_SOURCE
LDFLAGS  = -s ${LIBS}
