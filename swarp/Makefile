# swarp - simple pointer warp
#   (C)opyright MMVI Anselm R. Garbe

include config.mk

SRC = swarp.c
OBJ = ${SRC:.c=.o}

all: options swarp

options:
	@echo swarp build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"
	@echo "LD       = ${LD}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

swarp: ${OBJ}
	@echo LD $@
	@${LD} -o $@ ${OBJ} ${LDFLAGS}
	@strip $@

clean:
	@echo cleaning
	@rm -f swarp ${OBJ} swarp-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p swarp-${VERSION}
	@cp -R LICENSE Makefile README config.mk ${SRC} swarp-${VERSION}
	@tar -cf swarp-${VERSION}.tar swarp-${VERSION}
	@gzip swarp-${VERSION}.tar
	@rm -rf swarp-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f swarp ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/swarp

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/swarp

.PHONY: all options clean dist install uninstall
