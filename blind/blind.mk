.POSIX:

CONFIGFILE = config.mk

SCRIPTS    = $(SH_SCRIPTS) $(KSH_SCRIPTS)
MAN1       = $(BIN:=.1) $(SCRIPTS:=.1)
MAN7       = blind.7
BOBJ       = $(BIN:=.bo)
OBJ        = $(BIN:=.o) $(COMMON_OBJ)

DIST_MAN   = $(MAN1) $(MAN7)
DIST_MISC  = $(HDR) $(SRC) $(SCRIPTS) $(MISCFILES) $(EXAMPLEFILES)

DEPS       = build/common-files.mk blind.mk $(CONFIGFILE)
MCB_DEPS   = build/files.mk $(DEPS)

include $(CONFIGFILE)
include build/files.mk
include build/common-files.mk

all: $(BIN)
mcb: blind-mcb

$(BIN): $(@:=.o) $(COMMON_OBJ) $(DEPS)
.o:
	$(CC) -o $@ $< $(COMMON_OBJ) $(LDFLAGS)

$(OBJ): src/$(@:.o=:c) $(HDR) build/platform.h $(DEPS)
$(OBJ):
	$(CC) $(CFLAGS) -c -o $$$$.$@ src/$*.c && mv $$$$.$@ $@

build/blind-mcb.o: $(MCB_DEPS)
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(BOBJ): src/$(@:.bo=:c) $(HDR) build/platform.h $(DEPS)
$(BOBJ):
	$(CC) $(CFLAGS) -Dmain="$$(printf 'main_%s\n' $* | tr -- - _)" -c -o $@ src/$*.c

build/blind-mcb.c: $(MCB_DEPS)
	mkdir -p $(@D)
	printf '#include <%s.h>\n' stdio string > $@
	printf 'int main_%s(int argc, char *argv[]);\n' $(BIN) | tr -- - _ >> $@
	printf 'int main(int argc, char *argv[]) {\n' >> $@
	printf 'char *cmd = strrchr(*argv, '"'/'"');\n' >> $@
	printf 'cmd = cmd ? &cmd[1] : *argv;\n' >> $@
	set -e && for c in $(BIN); do \
		printf 'if (!strcmp(cmd, "%s"))\n\treturn main_%s(argc, argv);\n' "$$c" "$$c" | \
			sed '/^\t/s/-/_/g'; \
	done >> $@
	printf 'fprintf(stderr, "Invalid command: %%s\\n", cmd);\n' >> $@
	printf 'return 1;\n' >> $@
	printf '}\n' >> $@

blind-mcb: build/blind-mcb.o $(BOBJ) $(COMMON_OBJ) $(MCB_DEPS)
	$(CC) -o $@ build/blind-mcb.o $(BOBJ) $(COMMON_OBJ) $(LDFLAGS)

build/generate-macros: src/generate-macros.c $(DEPS)
	@mkdir -p -- $(@D)
	$(CC) $(CFLAGS) -o $@.$$$$ src/generate-macros.c $(LDFLAGS) && mv $@.$$$$ $@

src/../build/platform.h: build/platform.h
build/platform.h: build/generate-macros $(DEPS)
	@mkdir -p -- $(@D)
	build/generate-macros > $@.$$$$ && mv $@.$$$$ $@

install: all install-common
	cp -f -- $(BIN) "$(DESTDIR)$(PREFIX)/bin"
	cd -- "$(DESTDIR)$(PREFIX)/bin" && chmod 755 $(BIN)

install-mcb: mcb install-common
	for c in $(BIN); do \
		$(LN) -f -- blind-single-colour "$(DESTDIR)$(PREFIX)/bin/$$c"; done
	rm -f -- "$(DESTDIR)$(PREFIX)/bin/blind-single-colour"
	cp -f -- blind-mcb "$(DESTDIR)$(PREFIX)/bin/blind-single-colour"
	cd -- "$(DESTDIR)$(PREFIX)/bin" && chmod 755 -- blind-single-colour

install-common:
	mkdir -p -- "$(DESTDIR)$(PREFIX)/bin"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man1"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -f -- $(SCRIPTS) "$(DESTDIR)$(PREFIX)/bin"
	cd -- "$(DESTDIR)$(PREFIX)/bin" && sed -i '1s:bash$$:$(KORN_SHELL):' $(KSH_SCRIPTS)
	cd -- "$(DESTDIR)$(PREFIX)/bin" && chmod 755 -- $(SCRIPTS)
	set -e && for m in $(MAN); do \
		sed '1s/ blind$$/ " blind $(VERSION)"/g' \
		< "man/$$(basename $$m)" > "$(DESTDIR)$(MANPREFIX)/$$m"; \
	done
	cd -- "$(DESTDIR)$(MANPREFIX)" && chmod 644 $(MAN)

uninstall:
	cd -- "$(DESTDIR)$(PREFIX)/bin" && rm -f $(BIN) $(SCRIPTS)
	cd -- "$(DESTDIR)$(MANPREFIX)" && rm -f $(MAN)

dist:
	-rm -rf "blind-$(VERSION)"
	mkdir -p "blind-$(VERSION)"
	cd "blind-$(VERSION)" && mkdir -p src/util man $(EXAMPLEDIRS)
	cd man && cp $(DIST_MAN) "../blind-$(VERSION)/man"
	set -e && for s in $(DIST_MISC); do cp "$$s" "../blind-$(VERSION)/$$s"; done
	tar -cf "blind-$(VERSION).tar" "blind-$(VERSION)"
	gzip -9 "blind-$(VERSION).tar"
	rm -rf "blind-$(VERSION)"

.SUFFIXES:
.SUFFIXES: .o .bo .c

.PHONY: all mcb install install-mcb install-common uninstall dist
.PRECIOUS: $(COMMON_OBJ) build/platform.h
