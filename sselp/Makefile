# sselp - simple print selection

include config.mk

SRC = sselp.c
OBJ = ${SRC:.c=.o}

all: options sselp

options:
	@echo sselp build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

sselp: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f sselp ${OBJ} sselp-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sselp-${VERSION}
	@cp -R LICENSE Makefile README config.mk ${SRC} sselp-${VERSION}
	@tar -cf sselp-${VERSION}.tar sselp-${VERSION}
	@gzip sselp-${VERSION}.tar
	@rm -rf sselp-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sselp ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sselp

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sselp

.PHONY: all options clean dist install uninstall
