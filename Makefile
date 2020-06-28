NAME = ics2txt
VERSION = 0.2

W = -Wall -Wextra -std=c99 --pedantic
I = -Isrc
D = -D_POSIX_C_SOURCE=200811L -DVERSION='"${VERSION}"'
CFLAGS = $I $D $W -g
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

SRC = src/ical.c src/map.c src/util.c src/log.c
HDR = src/ical.h src/map.h src/util.h src/log.h
OBJ = ${SRC:.c=.o}
BIN = ics2tree

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${OBJ}: ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ}

clean:
	rm -rf *.o */*.o ${BIN} ${NAME}-${VERSION} *.gz

install:
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp bin/* $(BIN) ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}$(MANPREFIX)/man1
	cp doc/*.1 ${DESTDIR}$(MANPREFIX)/man1

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README Makefile doc bin ${SRC} ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz
