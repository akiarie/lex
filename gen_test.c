#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "parse.h"
#include "gen.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct token tokens[] = {
		{"vowel", "[aeiou]"},
		{"vowelb", "{vowel}b"},
	};
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
