BIN	= agenda
MAN1	= agenda.1

all:

README: $(MAN1)
	mandoc -T utf8 $(MAN1) | col -bx >$@

install:
	mkdir -p $(PREFIX)/bin
	cp $(BIN) $(PREFIX)/bin
	mkdir -p $(PREFIX)/share/man/man1
	cp $(MAN1) $(PREFIX)/share/man/man1
