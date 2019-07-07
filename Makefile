BIN	= ics2txt
MAN1	= ics2txt.1

all:

install:
	mkdir -p $(PREFIX)/bin
	cp $(BIN) $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp $(MAN1) $(PREFIX)/share/man/man1
