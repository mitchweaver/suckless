VERSION = 0.8.1

PREFIX = ${HOME}/usr/local
MANPREFIX = $(PREFIX)/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

INCS = -I$(X11INC) \
       `pkg-config --cflags fontconfig` \
       `pkg-config --cflags freetype2`

# LINUX 
LIBS = -L$(X11LIB) -lm -lrt -lX11 -lutil -lXft \
		   `pkg-config --libs fontconfig` \
	       	   `pkg-config --libs freetype2`

# OpenBSD
#LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft \
#       `pkg-config --libs fontconfig` \
#       `pkg-config --libs freetype2`

CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
STCFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

#CC = tcc -w
CC = gcc -w -O3 -pipe -std=c99 -fstack-protector-all -fpie
# CC = musl-gcc -w -O3 -pipe -std=c99 -fstack-protector-all -fpie
# CC = clang -w -O3 -pipe -std=c99 -fstack-protector-all -fpie -Wl,-pie -Wl,-z,relro
