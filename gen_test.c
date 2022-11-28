#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "parse.h"
#include "gen.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct { char *name; char *regex; } patterns[] = {
		{"vowel", "[aeiou]"},
		{"vowelb", "{vowel}b"},
	};
	struct fsmlist *l = NULL;
	for (int i = 0; i < LEN(patterns); i++) {
		struct fsm *s = fsm_fromstring(patterns[i].regex, l);
		l = fsmlist_append(l, patterns[i].name, s);
	}
	struct token tokens[] = {
		{"ab",		"{ /* action for ab */ }"},
		{"{vowelb}",	"{ /* action for vowelb */ }"},
		{"{vowel}",	"{ /* action for vowel */ }"},
	};
	struct lexer *lx = lexer_create("/* preamble */", "/* postamble */", l,
		tokens, LEN(tokens));
	/*lexer_destroy(lx);*/
	/*gen(tokens, 2, true, stdout);*/
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
