#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "lex.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct token tokens[] = {
		{"vowel", "[ae]"},
		{"vowelb", "{vowel}b"},
	};
	struct fsmlist *l = lexer(tokens, LEN(tokens));
	for (struct fsmlist *m = l; m != NULL; m = m->next) {
		fsm_destroy(m->s);
	}
	fsmlist_destroy(l);
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		run,
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}
}
