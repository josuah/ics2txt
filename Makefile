PREFIX	= /usr/local
BIN	= tical-*
MAN1	= tical.1

all:

install:
	mkdir -p $(PREFIX)/bin
	cp $(BIN) $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp $(MAN1) $(PREFIX)/share/man/man1
