NAME = ics2txt
VERSION = 0.1

BIN = ics2tsv tsv2tcal tcal2tsv tsv2ics ics2txt

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

all: ${BIN}

clean:
	rm -rf ${NAME}-${VERSION} *.gz

install:
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp $(BIN) ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}$(MANPREFIX)/man1
	cp doc/*.1 ${DESTDIR}$(MANPREFIX)/man1

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README Makefile doc ${BIN} ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz
