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
LIBS = -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft -lXcursor -lXrender `pkg-config --libs fontconfig` `pkg-config --libs freetype2`

# OpenBSD
#LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft -lXrender \
#		`pkg-config --libs fontconfig` \
#		`pkg-config --libs freetype2`

# NetBSD
# LIBS = -Wl,-R${X11LIB} -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft -lXrender `pkg-config --libs fontconfig` `pkg-config --libs freetype2`

STCFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS) -O3 -pipe -fomit-frame-pointer -fipa-pta -fno-semantic-interposition -fno-trapping-math -fno-math-errno -fasynchronous-unwind-tables  -O3 -fgraphite-identity -floop-nest-optimize -fdevirtualize-at-ltrans -fipa-pta -fno-semantic-interposition -flto=auto -fuse-linker-plugin -falign-functions=32
STLDFLAGS = $(LIBS) $(LDFLAGS) -fuse-ld=mold -Wl,-O1 -Wl,--as-needed
CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600 -O3 -pipe -fomit-frame-pointer -fipa-pta -fno-semantic-interposition -fno-trapping-math -fno-math-errno -fasynchronous-unwind-tables

CC = cc
