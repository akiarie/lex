CC = cc
OBJECTS = main.o thompson.o automata.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

thompson.o: thompson.c thompson.h
	$(CC) -c thompson.c

automata.o: automata.c automata.h thompson.h
	$(CC) -c automata.c

main.o: main.c thompson.h
	$(CC) -c main.c

thompson_test: thompson_test.c thompson.o
	$(CC) -o $@ thompson_test.c thompson.o

automata_test: automata_test.c automata.o thompson.o
	$(CC) -o $@ automata_test.c automata.o thompson.o

example: lex
	@cd examples; ../lex lex.l

check: thompson_test automata_test
	@./run-tests.sh

clean-tests: 
	@rm -f thompson_test

clean: clean-tests
	@rm -f lex $(OBJECTS) *.gch a.out
	@rm -rf *.dSYM
	@rm examples/*.yy.c

.PHONY: clean clean-tests
