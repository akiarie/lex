#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<assert.h>

#include "automata.h"
#include "parse.h"
#include "lex.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

static char *
dynamic_name(char *static_name)
{
	assert(static_name != NULL);
	int len = strlen(static_name) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s", static_name);
	return name;
}

void
run()
{
	char *pre = dynamic_name("/* preamble */");
	char *post = dynamic_name("/* postamble */");
	struct token tokens[] = {
		{"ws",		"[ ]"},
		{"vowel",	"[aeiou]"},
		{"vowelb",	"{vowel}b"},
	};
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
