OBJECTS = main.o thompson.o

lex: $(OBJECTS)
	cc -o lex $(OBJECTS)

thompson.o: thompson.c thompson.h
	cc -c thompson.c

main.o: main.c thompson.h
	cc -c main.c

clean:
	@rm -f lex $(OBJECTS)
