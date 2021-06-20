NAME = ics2txt
VERSION = 0.2

W = -Wall -Wextra -std=c99 --pedantic
D = -D_POSIX_C_SOURCE=200811L -D_BSD_SOURCE -DVERSION='"${VERSION}"'
CFLAGS = $D $W -g
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

SRC = ical.c base64.c util.c
HDR = ical.h base64.h util.h
OBJ = ${SRC:.c=.o}
AWK = tsv2ics.awk
BIN = ics2tree ics2tsv tsv2agenda
MAN1 = ics2txt.1 ics2tsv.1

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${AWK:.awk=}:
	cp $@.awk $@
	chmod +x $@

${OBJ}: ${HDR}
${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ}

clean:
	rm -rf *.o ${BIN} ${AWK:.awk} ${NAME}-${VERSION} *.gz

install: ${BIN} ${AWK:.awk=}
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp $(BIN) ${AWK:.awk=} ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}$(MANPREFIX)/man1
	cp ${MAN1} ${DESTDIR}$(MANPREFIX)/man1

dist: clean
	mkdir -p ${NAME}-${VERSION}
	cp -r README Makefile ${AWK} ${MAN1} ${SRC} ${NAME}-${VERSION}
	tar -cf - ${NAME}-${VERSION} | gzip -c >${NAME}-${VERSION}.tar.gz

.SUFFIXES: .awk
.PHONY: ${AWK}
