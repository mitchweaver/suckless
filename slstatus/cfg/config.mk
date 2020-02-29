VERSION = 0

PREFIX = ${HOME}/.local
MANPREFIX = $(PREFIX)/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

CPPFLAGS = -I$(X11INC) -D_DEFAULT_SOURCE
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -O2 -pipe -fstack-protector-all
LDFLAGS  = -L$(X11LIB) -s
LDLIBS   = -lX11

CC = cc
