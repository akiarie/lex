#!/bin/sh
cat automata.h automata_util.c automata_fsm.c automata.c lex.h lex.c \
	| grep -v '#include "automata.h"' - \
	| grep -v '#include "automata_fsm.c"' - \
	| grep -v '#include "automata_util.c"' - \
	| grep -v '#include "lex.h"' - \
	| xxd -name lex_gen_file -i -
