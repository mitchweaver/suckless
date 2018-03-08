.POSIX:

include config.mk

SRC = surf.c
OBJ = $(SRC:.c=.o)

all: options surf

options:
	@echo surf build options:
	@echo "CFLAGS   = $(SURF_CFLAGS)"
	@echo "LDFLAGS  = $(SURF_LDFLAGS)"
	@echo "CC       = $(CC)"

.c.o:
	@echo CC -c $<
	@$(CC) $(SURF_CFLAGS) -c $<

$(OBJ): config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

surf: $(OBJ)
	@echo CC -o $@
	@$(CC) $(SURF_CFLAGS) -o $@ $(OBJ) $(SURF_LDFLAGS)

clean:
	@echo cleaning
	@rm -f surf $(OBJ)

distclean: clean
	@echo cleaning dist
	@rm -f config.h surf-$(VERSION).tar.gz

dist: distclean
	@echo creating dist tarball
	@mkdir -p surf-$(VERSION)
	@cp -R LICENSE Makefile config.mk config.def.h README \
	    surf-open.sh arg.h TODO.md surf.png \
	    surf.1 $(SRC) surf-$(VERSION)
	@tar -cf surf-$(VERSION).tar surf-$(VERSION)
	@gzip surf-$(VERSION).tar
	@rm -rf surf-$(VERSION)

install: all
	@echo installing executable file to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f surf $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/surf
	@echo installing manual page to $(DESTDIR)$(MANPREFIX)/man1
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	@sed "s/VERSION/$(VERSION)/g" < surf.1 > $(DESTDIR)$(MANPREFIX)/man1/surf.1
	@chmod 644 $(DESTDIR)$(MANPREFIX)/man1/surf.1

uninstall:
	@echo removing executable file from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin/surf
	@echo removing manual page from $(DESTDIR)$(MANPREFIX)/man1
	@rm -f $(DESTDIR)$(MANPREFIX)/man1/surf.1

.PHONY: all options clean dist install uninstall
