VERSION = 0.8.2

PREFIX = ${HOME}/.local
MANPREFIX = $(PREFIX)/share/man

# OpenBSD / Linux
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# NetBSD
# X11INC = /usr/X11R7/include
# X11LIB = /usr/X11R7/lib

INCS = -I$(X11INC) \
       `pkg-config --cflags fontconfig` \
       `pkg-config --cflags freetype2` \
	   -I${HOME}/.cache/themes

# Linux
# CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
# OpenBSD
CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -D_BSD_SOURCE
# NetBSD
# CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -D_NETBSD_SOURCE

LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft -lXrender\
		`pkg-config --libs fontconfig` \
		`pkg-config --libs freetype2`
# Linux
# LIBS = -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft -lXrender `pkg-config --libs fontconfig` `pkg-config --libs freetype2`

# OpenBSD
LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft -lXrender \
		`pkg-config --libs fontconfig` \
		`pkg-config --libs freetype2`

# NetBSD
# LIBS = -Wl,-R${X11LIB} -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft -lXrender `pkg-config --libs fontconfig` `pkg-config --libs freetype2`

STCFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

CC = cc
