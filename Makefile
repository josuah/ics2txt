NAME = ics2txt
VERSION = 0.2

W = -Wall -Wextra -std=c99 --pedantic
D = -D_POSIX_C_SOURCE=200811L -DVERSION='"${VERSION}"'
CFLAGS = $D $W -g
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

SRC = ical.c base64.c util.c
HDR = ical.h base64.h util.h
OBJ = ${SRC:.c=.o}
BIN = ics2tree ics2tsv
MAN1 = ics2txt.1
MAN5 = tcal.5

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${OBJ}: ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ}

clean:
	rm -rf *.o ${BIN} ${NAME}-${VERSION} *.gz

install:
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp bin/* $(BIN) ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}$(MANPREFIX)/man1
	cp ${MAN1} ${DESTDIR}$(MANPREFIX)/man1
	mkdir -p ${DESTDIR}$(MANPREFIX)/man5
	cp ${MAN5} ${DESTDIR}$(MANPREFIX)/man5

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README Makefile bin ${SRC} ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz
