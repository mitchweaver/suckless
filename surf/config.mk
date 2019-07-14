VERSION = 2.0

PREFIX = ${HOME}/usr/local
MANPREFIX = $(PREFIX)/share/man
LIBPREFIX = $(PREFIX)/lib/surf

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

GTKINC = `pkg-config --cflags gtk+-3.0 gcr-3 webkit2gtk-4.0`
GTKLIB = `pkg-config --libs gtk+-3.0 gcr-3 webkit2gtk-4.0`

INCS = -I$(X11INC) $(GTKINC) 
LIBS = -L$(X11LIB) -lX11 $(GTKLIB) -lgthread-2.0

CPPFLAGS = -DVERSION=\"${VERSION}\" -DWEBEXTDIR=\"${LIBPREFIX}\" \
           -D_DEFAULT_SOURCE -DGCR_API_SUBJECT_TO_CHANGE
SURF_CFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS)
SURF_LDFLAGS = $(LIBS) $(LDFLAGS)

CC = gcc -w -O3 -pipe -std=c99 -fstack-protector-all -fpie
#CC = musl-gcc -w -O3 -pipe -std=c99 -fstack-protector-all -fpie
#CC = tcc -w
