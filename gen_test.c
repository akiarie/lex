#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "parse.h"
#include "gen.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

static char *
dynamic_name(char *static_name)
{
	int len = strlen(static_name) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s", static_name);
	return name;
}

void
run()
{
	struct pattern patterns[] = {
		{"vowel", "[aeiou]"},
		{"vowelb", "{vowel}b"},
	};
	struct token tokens[] = {
		{true,	"ab",		"{ /* action for ab */ }"},
		{false,	"vowelb",	"{ /* action for vowelb */ }"},
		{false,	"vowel",	"{ /* action for vowel */ }"},
	};
	struct lexer *lx = lexer_create(dynamic_name("/* preamble */"),
		dynamic_name("/* postamble */"), patterns, LEN(patterns),
		tokens, LEN(tokens));
	gen(stdout, lx, true);
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
