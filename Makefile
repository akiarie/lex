CC = cc -g
OBJECTS = main.o gen.o thompson.o automata.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

thompson.o: thompson.c thompson.h
	$(CC) -c thompson.c

AUTOMATA_SRC = automata.c automata_fsm.c automata_util.c

automata.o: automata.h $(AUTOMATA_SRC) thompson.h
	$(CC) -c automata.c

# avails the automata library so that util can generate code that refers to it
lex_gen.c: automata.h $(AUTOMATA_SRC) lex.h lex.c
	@./gen-preamble.sh > $@

gen.o: gen.c lex_gen.c gen.h lex.h
	$(CC) -c gen.c

main.o: main.c gen.h thompson.h
	$(CC) -c main.c

## only used by tests
lex.o: lex.c automata.h thompson.h
	$(CC) -c lex.c

thompson_test: thompson_test.c thompson.o
	$(CC) -o $@ thompson_test.c thompson.o

automata_test: automata_test.c lex.o automata.o thompson.o
	$(CC) -o $@ automata_test.c lex.o automata.o thompson.o

gen_test: gen_test.c lex_gen.c gen.o
	$(CC) -o $@ gen_test.c gen.o

lex_test: lex_test.c lex.o automata.o thompson.o
	$(CC) -o $@ lex_test.c lex.o automata.o thompson.o

example: lex
	@cd examples; ../lex lex.l

check: thompson_test automata_test gen_test lex_test
	@./run-tests.sh

clean-tests: 
	@rm -f *_test

clean: clean-tests
	@rm -f lex $(OBJECTS) lex_gen.o *.gch a.out
	@rm -rf *.dSYM
	@rm -rf examples/*.yy.c
	@rm -rf lex_gen.c

.PHONY: clean clean-tests
