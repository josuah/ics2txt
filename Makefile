PREFIX	= /usr/local
BIN	= ics2tsv tsv2tcal tcal2tsv tsv2ics ics2txt
MAN1	= ics2tsv.1

all:

install:
	mkdir -p $(PREFIX)/bin
	cp $(BIN) $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp $(MAN1) $(PREFIX)/share/man/man1
