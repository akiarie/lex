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
	struct fsmlist *l = fsmlist_append(NULL, "", nfa);
	for (char *c = cs->input; *c != '\0'; c++) {
		l = fsmlist_sim(l, *c);
		if (l == NULL) {
			break;
		}
	}
	bool response = fsmlist_accepting(l) == cs->shouldaccept;
	/*fsmlist_destroy(l);*/
	return response;
}

void run_cases(struct fsmcase cases[], int len, struct fsm *s)
{
	for (int i = 0; i < len; i++) {
		printf("Running '%s' with \n", cases[i].input);
		fsm_print(s);
		if (!runfsmcase(s, &cases[i])) {
			fprintf(stderr, "'%s' case failed\n", cases[i].input);
			exit(1);
		}
	}
}

void
simple_expressions()
{
	struct fsm *s = util_fsm_fromstring("ab", NULL);
	struct fsmcase cases[] = {
		{false,	"a"},
		{true,	"ab"},
	};
	run_cases(cases, LEN(cases), s);
	fsm_destroy(s);
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
