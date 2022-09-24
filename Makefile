INCLUDE=include
SRC=src
CC=cc

HEADERS= $(INCLUDE)/thompson.h

OBJECTS= $(SRC)/thompson.o $(SRC)/main.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

include tests/Makefile

$(SRC)/%.o: $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@rm -f lex $(OBJECTS)
