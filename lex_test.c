#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "lex.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct token tokens[] = {
		{"vowel", "[ae]"},
		{"vowelb", "{vowel}b"},
	};
	/*util_gen(&tokens[0], 2, NULL);*/
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
