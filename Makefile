CC = cc
OBJECTS = main.o util.o thompson.o automata.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

thompson.o: thompson.c thompson.h
	$(CC) -c thompson.c

automata.o: automata.c automata.h thompson.h
	$(CC) -c automata.c

# avails the automata library so that util can generate code that refers to it
util_gen.c: 
	@xxd -n util_automata_h_file -i automata.h >> $@
	@grep -v "#include \"automata.h\"" automata.c | xxd -name util_automata_c_file -i >> $@

util.o: util.c util_gen.c util.h automata.h thompson.h
	$(CC) -c util.c util_gen.c

main.o: main.c util.h thompson.h
	$(CC) -c main.c

thompson_test: thompson_test.c thompson.o
	$(CC) -o $@ thompson_test.c thompson.o

automata_test: automata_test.c util.o automata.o thompson.o
	$(CC) -o $@ automata_test.c util.o automata.o thompson.o

example: lex
	@cd examples; ../lex lex.l

check: thompson_test automata_test
	@./run-tests.sh

clean-tests: 
	@rm -f thompson_test

clean: clean-tests
	@rm -f lex $(OBJECTS) *.gch a.out
	@rm -rf *.dSYM
	@rm -rf examples/*.yy.c
	@rm -rf util_gen.c

.PHONY: clean clean-tests
