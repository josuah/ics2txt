NAME = ics2txt
VERSION = 1.0
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

D = -D_POSIX_C_SOURCE=200811L -D_BSD_SOURCE -DVERSION='"${VERSION}"'
CFLAGS = -g -Wall -Wextra -std=c99 --pedantic -fPIC $D
LDFLAGS = -static

SRC = ical.c base64.c util.c
HDR = ical.h base64.h util.h
OBJ = ${SRC:.c=.o}
AWK = tsv2ics.awk
BIN = ics2tree ics2tsv tsv2agenda
MAN1 = ics2tsv.1 tsv2ics.1 tsv2agenda.1

all: ${BIN}

.c.o:
	${CC} -c ${CFLAGS} -o $@ $<

${AWK:.awk=}:
	cp $@.awk $@
	chmod +x $@

${OBJ} ${BIN:=.o}: Makefile ${HDR}

${BIN}: ${OBJ} ${BIN:=.o}
	${CC} ${LDFLAGS} -o $@ $@.o ${OBJ} ${LIB}

clean:
	rm -rf *.o ${BIN} ${AWK:.awk=} ${NAME}-${VERSION} *.tgz

install: ${BIN} ${AWK:.awk=}
	mkdir -p ${DESTDIR}$(PREFIX)/bin
	cp -f $(BIN) ${AWK:.awk=} ${DESTDIR}$(PREFIX)/bin
	mkdir -p ${DESTDIR}$(MANPREFIX)/man1
	cp -f ${MAN1} ${DESTDIR}$(MANPREFIX)/man1

dist:
	git archive v${VERSION} --prefix=${NAME}-${VERSION}/ \
	| gzip >${NAME}-${VERSION}.tgz

site:
	notmarkdown README.md | notmarkdown-html | cat .head.html - >index.html
	notmarkdown README.md | notmarkdown-gph | cat .head.gph - >index.gph
	mkdir -p man
	mandoc -Thtml -Ofragment ${MAN1} | cat .head.html - >man/index.html
	mandoc -Tutf8 ${MAN1} | ul -t dumb >man/index.gph
	sed -i "s/NAME/${NAME}/g; s/VERSION/${VERSION}/g" index.* man/index.*

.SUFFIXES: .awk
.PHONY: ${AWK}
