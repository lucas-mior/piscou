PREFIX ?= /usr/local

src = main.c
headers = config.h

ldlibs = $(LDLIBS) -lmagic

all: release

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

clang: CC=clang
clang: CFLAGS += -Weverything -Wno-unsafe-buffer-usage
clang: clean
clang: piscou

CFLAGS += -std=c99 -D_DEFAULT_SOURCE
CFLAGS += -Wall -Wextra

release: CFLAGS += -O2 -flto
release: piscou

debug: CFLAGS += -g
debug: CFLAGS += -DPISCOU_DEBUG -fsanitize=undefined
debug: CFLAGS += -Wno-format-zero-length
debug: clean
debug: piscou

piscou: $(src) $(headers) Makefile
	ctags --kinds-C=+l *.h *.c
	vtags.sed tags > .tags.vim
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(src) $(ldlibs)

install: all
	install -Dm755 piscou   ${DESTDIR}${PREFIX}/bin/piscou
	install -Dm644 piscou.1 ${DESTDIR}${PREFIX}/man/man1/piscou.1
	install -Dm644 LICENSE  ${DESTDIR}${PREFIX}/share/licenses/${pkgname}/LICENSE
uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/piscou
	rm -f ${DESTDIR}${PREFIX}/share/man/man1/piscou.1
	rm -f ${DESTDIR}${PREFIX}/share/licenses/${pkgname}/LICENSE

clean:
	rm -f piscou
