INCLUDE=include
SRC=src
CC=cc

HEADERS= \
	$(INCLUDE)/thompson.h

OBJECTS= \
	$(SRC)/thompson.o \
	$(SRC)/main.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

include tests/Makefile

%.o: $(HEADERS)
	$(CC) -c -o $@ $<

clean:
	@rm -f lex $(OBJECTS)
