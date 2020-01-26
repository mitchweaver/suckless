VERSION = 0.8.1

PREFIX = ${HOME}/.local
MANPREFIX = $(PREFIX)/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I$(X11INC) \
       `pkg-config --cflags fontconfig` \
       `pkg-config --cflags freetype2`

# -------- Linux ---------------------
#CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
#LIBS = -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft \
#		   `pkg-config --libs fontconfig` \
#	       	   `pkg-config --libs freetype2`


# --------- OpenBSD: --------------
CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft \
       `pkg-config --libs fontconfig` \
       `pkg-config --libs freetype2`


STCFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

CC = gcc -w -O3 -pipe -fstack-protector-all -fpie
