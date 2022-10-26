#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "util.h"

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

void run_cases(struct fsmcase cases[], int len, struct fsm *s)
{
	for (int i = 0; i < len; i++) {
		if (!runfsmcase(s, &cases[i])) {
			fprintf(stderr, "'%s' case failed\n", cases[i].input);
			exit(1);
		}
	}
}

void
simple_expressions()
{
	/*struct fsm *s = util_fsm_fromstring("a([bcg-z0-3])*d", NULL);*/
	struct fsm *s = util_fsm_fromstring("a*d", NULL);
	fsm_print(s);
	struct fsmcase cases[] = {
		/*{false, "hello, world!"},*/
		{true,  "ad"},
		/*{true,  "abd"},*/
		/*{true,  "acd"},*/
		/*{true,  "abcccccccbbcd"},*/
		/*{false, "abcefd"},*/
		/*{true,  "abcghijpqrwzzcd"},*/
		/*{false, "abcghi9jpqrwzzcd"},*/
		/*{true,  "abcghi123jpqrwzzcd"},*/
		/*{false, "a"},*/
		/*{false, "abcgh"},*/
	};
	run_cases(cases, LEN(cases), s);
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
	struct fsmlist *list = NULL;
	for (int i = 0; i < LEN(patterns); i++) {
		struct fsm *s = util_fsm_fromstring(patterns[i].regex, list);
		list = fsmlist_append(list, patterns[i].name, s);
	}
	struct fsmlist *m = list;
	for (; strcmp("vword", m->name) != 0; m = m->next) {}
	struct fsm *vword = m->s;
	struct fsmcase vcases[] = {
		{false, "baaaooaoa"},
		{true,  "abwerqasd"},
		{true,  "ebwerqasd"},
		{true,  "ibwerqasd"},
	};
	run_cases(vcases, LEN(vcases), vword);
	for (m = list; strcmp("cword", m->name) != 0; m = m->next) {}
	struct fsm *cword = m->s;
	struct fsmcase ccases[] = {
		{false, "abcefd"},
		{false, "ebcghi9jpqrwzzcd"},
		{true,  "basdfaue"},
		{true,  "gasdfaue"},
		{true,  "hasdfaue"},
	};
	run_cases(ccases, LEN(ccases), cword);
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		simple_expressions,
		/*piglatin,*/
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}

}
