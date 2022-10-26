#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "util.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct { char *name; char *regex; } patterns[] = {
		{"vowel", "ab"},
	};
	struct fsmlist *list = NULL;
	for (int i = 0; i < LEN(patterns); i++) {
		struct fsm *s = util_fsm_fromstring(patterns[i].regex, list);
		list = fsmlist_append(list, patterns[i].name, s);
	}
	/*fsm_print(list->s);*/
	/*util_gen(list, "lex_automaton", NULL);*/
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		/*run,*/
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}
}
