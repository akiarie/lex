#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

struct fsmcase {
	bool shouldaccept;
	char *input;
};

bool
runfsmcase(struct fsm *nfa, struct fsmcase *cs)
{
	struct fsm *next = nfa;
	for (char *c = cs->input; *c != '\0'; c++) {
		next = fsm_sim(next, *c);
		if (next == NULL) {
			break;
		}
	}
	return fsm_isaccepting(next) == cs->shouldaccept;
}

void
run()
{
	struct fsm* nfa = automata_string_conv("a(b|c)d");
	struct fsmcase cases[] = {
		{false, "hello, world!"},
		{true,  "abd"},
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		if (!runfsmcase(nfa, &cases[i])) {
			fprintf(stderr, "'%s' case failed\n", cases[i].input);
			exit(1);
		}
	}
}

typedef void (*testcase)(void);

int
main()
{
	testcase cases[] = {
		run,
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		cases[i]();
	}
}
