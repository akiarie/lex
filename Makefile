CC = cc
OBJECTS = main.o thompson.o

lex: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

thompson.o: thompson.c thompson.h
	$(CC) -c thompson.c

main.o: main.c thompson.h
	$(CC) -c main.c

# test
thompson_test: thompson_test.c thompson.o
	$(CC) -o $@ thompson_test.c thompson.o

check: thompson_test
	@./run-tests.sh

clean-tests: 
	@rm -f thompson_test

clean: clean-tests
	@rm -f lex $(OBJECTS) *.gch a.out
	@rm -rf *.dSYM

.PHONY: clean clean-tests
