# See LICENSE file for copyright and license details
# quark - simple web server
.POSIX:

include config.mk

all: quark

quark: quark.c arg.h config.h config.mk
	$(CC) -o $@ $(CPPFLAGS) $(CFLAGS) quark.c $(LDFLAGS)

config.h:
	cp config.def.h $@

clean:
	rm -f quark

dist:
	rm -rf "quark-$(VERSION)"
	mkdir -p "quark-$(VERSION)"
	cp -R LICENSE Makefile arg.h config.def.h config.mk quark.1 \
		quark.c "quark-$(VERSION)"
	tar -cf - "quark-$(VERSION)" | gzip -c > "quark-$(VERSION).tar.gz"
	rm -rf "quark-$(VERSION)"

install: all
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f quark "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/quark"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp quark.1 "$(DESTDIR)$(MANPREFIX)/man1/quark.1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/quark.1"

uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/bin/quark"
	rm -f "$(DESTDIR)$(MANPREFIX)/man1/quark.1"
