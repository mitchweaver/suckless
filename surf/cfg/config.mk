VERSION = 2.0

PREFIX = ${HOME}/usr/local
MANPREFIX = $(PREFIX)/share/man
LIBPREFIX = $(PREFIX)/lib
LIBDIR = $(LIBPREFIX)/surf

X11INC = `pkg-config --cflags x11`
X11LIB = `pkg-config --libs x11`

GTKINC = `pkg-config --cflags gtk+-3.0 gcr-3 webkit2gtk-4.0`
GTKLIB = `pkg-config --libs gtk+-3.0 gcr-3 webkit2gtk-4.0`
WEBEXTINC = `pkg-config --cflags webkit2gtk-4.0 webkit2gtk-web-extension-4.0`
WEBEXTLIBS = `pkg-config --libs webkit2gtk-4.0 webkit2gtk-web-extension-4.0`

INCS = $(X11INC) $(GTKINC)
LIBS = $(X11LIB) $(GTKLIB) -lgthread-2.0

CPPFLAGS = -DVERSION=\"$(VERSION)\" -DWEBEXTDIR=\"$(LIBDIR)\" \
           -D_DEFAULT_SOURCE -DGCR_API_SUBJECT_TO_CHANGE

CFLAGS = -O2 -fstack-protector-all -std=c99 -pedantic -Wall
SURFCFLAGS = $(INCS) $(CPPFLAGS) -fPIC
WEBEXTCFLAGS = -fPIC $(WEBEXTINC)

CC = cc
