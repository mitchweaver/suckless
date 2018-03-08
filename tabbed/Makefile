# tabbed - tabbing interface
# See LICENSE file for copyright and license details.

include config.mk

SRC = tabbed.c xembed.c
OBJ = ${SRC:.c=.o}
BIN = ${OBJ:.o=}

all: options ${BIN}

options:
	@echo tabbed build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

.o:
	@echo CC -o $@
	@${CC} -o $@ $< ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f ${BIN} ${OBJ} tabbed-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p tabbed-${VERSION}
	@cp -R LICENSE Makefile README config.def.h config.mk \
		tabbed.1 arg.h ${SRC} tabbed-${VERSION}
	@tar -cf tabbed-${VERSION}.tar tabbed-${VERSION}
	@gzip tabbed-${VERSION}.tar
	@rm -rf tabbed-${VERSION}

install: all
	@echo installing executable files to ${DESTDIR}${PREFIX}/bin
	@mkdir -p "${DESTDIR}${PREFIX}/bin"
	@cp -f ${BIN} "${DESTDIR}${PREFIX}/bin"
	@chmod 755 "${DESTDIR}${PREFIX}/bin/tabbed"
	@echo installing manual pages to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	@sed "s/VERSION/${VERSION}/g" < tabbed.1 > "${DESTDIR}${MANPREFIX}/man1/tabbed.1"
	@chmod 644 "${DESTDIR}${MANPREFIX}/man1/tabbed.1"
	@sed "s/VERSION/${VERSION}/g" < xembed.1 > "${DESTDIR}${MANPREFIX}/man1/xembed.1"
	@chmod 644 "${DESTDIR}${MANPREFIX}/man1/xembed.1"

uninstall:
	@echo removing executable files from ${DESTDIR}${PREFIX}/bin
	@rm -f "${DESTDIR}${PREFIX}/bin/tabbed"
	@rm -f "${DESTDIR}${PREFIX}/bin/xembed"
	@echo removing manual pages from ${DESTDIR}${MANPREFIX}/man1
	@rm -f "${DESTDIR}${MANPREFIX}/man1/tabbed.1"
	@rm -f "${DESTDIR}${MANPREFIX}/man1/xembed.1"

.PHONY: all options clean dist install uninstall
