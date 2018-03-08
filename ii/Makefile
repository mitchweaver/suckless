# See LICENSE file for copyright and license details.
include config.mk

SRC = ii.c strlcpy.c
OBJ = ${SRC:.c=.o}

all: ii

options:
	@echo ii build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.o:
	$(LD) -o $@ $< $(LDFLAGS)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

ii: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${OBJ}: arg.h

install: all
	mkdir -p ${DESTDIR}${DOCDIR}
	mkdir -p ${DESTDIR}${BINDIR}
	mkdir -p ${DESTDIR}${MAN1DIR}
	install -d ${DESTDIR}${BINDIR} ${DESTDIR}${MAN1DIR}
	install -m 644 CHANGES README FAQ LICENSE ${DESTDIR}${DOCDIR}
	install -m 775 ii ${DESTDIR}${BINDIR}
	sed "s/VERSION/${VERSION}/g" < ii.1 > ${DESTDIR}${MAN1DIR}/ii.1
	chmod 644 ${DESTDIR}${MAN1DIR}/ii.1

uninstall: all
	rm -f ${DESTDIR}${MAN1DIR}/ii.1 \
		${DESTDIR}${BINDIR}/ii
	rm -rf ${DESTDIR}${DOCDIR}

dist: clean
	mkdir -p ii-${VERSION}
	cp -R Makefile CHANGES README FAQ LICENSE strlcpy.c arg.h \
		config.mk ii.c ii.1 ii-${VERSION}
	tar -cf ii-${VERSION}.tar ii-${VERSION}
	gzip ii-${VERSION}.tar
	rm -rf ii-${VERSION}

clean:
	rm -f ii *.o
