#!/bin/sh
cat thompson.h thompson.c automata.h automata_util.c automata_fsm.c automata.c \
	parse.h parse.c \
	| grep -v '#include "\(thompson\|automata\|parse\).h\|automata_\(fsm\|util\).c"' - \
	| xxd -name lex_gen_file -i -
