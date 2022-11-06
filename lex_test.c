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
		{"ws", "[ ]"},
		{"vowel", "[ae]"},
		{"vowelb", "{vowel}b"},
	};
	struct lexer *lx = lexer_create(tokens, LEN(tokens), "abra cada bra");
	fsmlist_print(lx->l);
	lexer_destroy(lx);
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
