.POSIX:

CONFIGFILE = config.mk
include files.mk
include $(CONFIGFILE)

SH_SCRIPTS =\
	blind-rotate-90\
	blind-rotate-180\
	blind-rotate-270

KSH_SCRIPTS =\
	blind-primary-key

COMMON_OBJ =\
	util.o\
	stream.o

HDR =\
	arg.h\
	common.h\
	define-functions.h\
	stream.h\
	util.h\
	util/to.h\
	util/jobs.h\
	util/emalloc.h\
	util/eopen.h\
	util/endian.h\
	util/colour.h\
	util/io.h\
	util/efflush.h\
	util/efunc.h\
	util/eprintf.h\
	util/fshut.h\
	video-math.h

MISCFILES =\
	Makefile\
	config.mk\
	blind.mk\
	rules.mk\
	LICENSE\
	README\
	TODO\
	src/generate-macros.c

EXAMPLEFILES =\
	inplace-flop/Makefile\
	reverse/Makefile\
	split/Makefile

MAN7 =\
	blind

all: build/files.mk build/common-files.mk
	@make -f blind.mk $@

build/files.mk: files.mk
	mkdir -p -- $(@D)
	(	printf 'BIN =' && \
		printf '\\\n\t%s' $(BIN) && \
		printf '\n\nSRC =' && \
		printf '\\\n\tsrc/%s' $(BIN:=.c) $(COMMON_OBJ:.o=.c) && \
		printf '\n\nEXAMPLEDIRS =' && \
		printf '\\\n\texamples/%s' $(EXAMPLEFILES) | sed 's|/[^/\\]*\(\\*\)$$|\1|' | uniq && \
		printf '\n\nEXAMPLEFILES =' && \
		printf '\\\n\texamples/%s' $(EXAMPLEFILES) && \
		printf '\n\nMISCFILES =' && \
		printf '\\\n\t%s' $(MISCFILES) && \
		printf '\n\nSH_SCRIPTS =' && \
		printf '\\\n\t%s' $(SH_SCRIPTS) && \
		printf '\n\nKSH_SCRIPTS =' && \
		printf '\\\n\t%s' $(KSH_SCRIPTS) && \
		printf '\n\nMAN =' && \
		printf '\\\n\tman1/%s.1' $(BIN) $(SH_SCRIPTS) $(KSH_SCRIPTS) && \
		printf '\\\n\tman7/%s.7' $(MAN7) && \
		printf '\n' \
	) > $@.$$$$ && mv $@.$$$$ $@

build/common-files.mk: Makefile
	mkdir -p -- $(@D)
	(	printf 'HDR =' && \
		printf '\\\n\tsrc/%s' $(HDR) && \
		printf '\n\nCOMMON_OBJ =' && \
		printf '\\\n\t%s' $(COMMON_OBJ) && \
		printf '\n' \
	) > $@.$$$$ && mv $@.$$$$ $@

clean:
	-rm -rf -- $(BIN) build *.o *.a *.bo blind-mcb
	-rm -rf -- blind-$(VERSION).tar.gz "blind-$(VERSION)"

.DEFAULT:
	@make build/files.mk build/common-files.mk
	@make -f blind.mk $@

.PHONY: all clean
