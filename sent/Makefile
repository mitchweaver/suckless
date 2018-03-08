include config.mk

SRC = sent.c drw.c util.c
OBJ = ${SRC:.c=.o}

all: options sent

options:
	@echo sent build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

config.h:
	cp config.def.h config.h

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

sent: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

cscope: ${SRC} config.h
	@echo cScope
	@cscope -R -b || echo cScope not installed

clean:
	@echo cleaning
	@rm -f sent ${OBJ} sent-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sent-${VERSION}
	@cp -R LICENSE Makefile config.mk config.def.h ${SRC} sent-${VERSION}
	@tar -cf sent-${VERSION}.tar sent-${VERSION}
	@gzip sent-${VERSION}.tar
	@rm -rf sent-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sent ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sent

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sent

.PHONY: all options clean dist install uninstall cscope
