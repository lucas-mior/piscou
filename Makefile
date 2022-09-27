PREFIX ?= /usr/local

objs = main.o util.o

ldlibs = $(LDLIBS) -lmagic

all: piscou

.PHONY: all clean install uninstall
.SUFFIXES:
.SUFFIXES: .c .o

CC=clang

piscou: $(objs)
	ctags --kinds-C=+l *.h *.c
	vtags.sed tags > .tags.vim
	$(CC) -Weverything $(cflags) $(LDFLAGS) -O2 -s -o $@ $(objs) $(ldlibs)

$(objs): Makefile piscou.h

main.o: piscou.h
util.o: piscou.h

.c.o:
	$(CC) -Weverything $(cflags) $(cppflags) -s -O2 -c -o $@ $<

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f piscou ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/piscou

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/piscou

clean:
	rm -f *.o *~ piscou
