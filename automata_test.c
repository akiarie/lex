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
simple_expressions()
{
	struct fsm *s = automata_fromstring("a([bcg-z0-3])*d", NULL);
	struct fsmcase cases[] = {
		{false, "hello, world!"},
		{true,  "ad"},
		{true,  "abd"},
		{true,  "acd"},
		{true,  "abcccccccbbcd"},
		{false, "abcefd"},
		{true,  "abcghijpqrwzzcd"},
		{false, "abcghi9jpqrwzzcd"},
		{true,  "abcghi123jpqrwzzcd"},
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		if (!runfsmcase(s, &cases[i])) {
			fprintf(stderr, "'%s' case failed\n", cases[i].input);
			exit(1);
		}
	}
}

void
piglatin()
{
	struct { char *name; char *regex; } patterns[] = {
		{"letter",	"[A-Za-z]"},
		{"vowel",	"[AEIOUaeiou]"},
		{"cons",	"[BCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz]"},
		{"vword",	"{vowel}{letter}*"},
		{"cword",	"{cons}{letter}*"},
	};
	struct fsmlist *l = NULL;
	for (int i = 0; i < LEN(patterns); i++) {
		struct fsm *s = automata_fromstring(patterns[i].regex, l);
		l = fsmlist_append(l, patterns[i].name, s);
	}
}

typedef void (*testcase)(void);

int
main()
{
	testcase cases[] = {
		simple_expressions,
		piglatin,
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		cases[i]();
	}
}
