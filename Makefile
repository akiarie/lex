CC = cc -g
OBJECTS = gen.o thompson.o automata.o parse.o

lex: main.o
	$(CC) -o $@ main.o $(OBJECTS)

thompson.o: thompson.c thompson.h
	$(CC) -c thompson.c

AUTOMATA_SRC = automata.c automata_fsm.c automata_util.c

automata.o: automata.h $(AUTOMATA_SRC) thompson.h
	$(CC) -c automata.c

parse.o: parse.c parse.h automata.h
	$(CC) -c parse.c

# avails the automata library so that util can generate code that refers to it
lex_gen.c: automata.h gen-preamble.sh $(AUTOMATA_SRC)
	@./gen-preamble.sh > $@

gen.o: gen.c lex_gen.c gen.h
	$(CC) -c gen.c

main.o: main.c $(OBJECTS)
	$(CC) -c main.c

thompson_test: thompson_test.c thompson.o
	$(CC) -o $@ thompson_test.c thompson.o

automata_test: automata_test.c $(OBJECTS)
	$(CC) -o $@ automata_test.c $(OBJECTS)

gen_test: gen_test.c lex_gen.c gen_test_gen.c $(OBJECTS)
	$(CC) -o $@ gen_test.c $(OBJECTS)

parse_test: parse_test.c $(OBJECTS)
	$(CC) -o $@ parse_test.c $(OBJECTS)

example: lex
	@cd examples; ../lex lex.l

check: thompson_test automata_test gen_test parse_test
	@./run-tests.sh

clean-tests: 
	@rm -f *_test

clean: clean-tests
	@rm -f lex $(OBJECTS) main.o lex_gen.o *.gch a.out
	@rm -rf *.dSYM
	@rm -rf *.yy.c examples/*.yy.c
	@rm -rf lex_gen.c

.PHONY: clean clean-tests
