#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<assert.h>

#include "automata.h"
#include "lex.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

struct fsmcase {
	bool shouldaccept;
	char *input;
};

bool fsm_simulate(struct fsm *s, char *input)
{
	for (char *c = input; *c != '\0'; c++) {
		s = fsm_sim(s, *c);
		assert(s != NULL);
	}
	return s->accepting;
}

bool
runfsmcase(struct fsm *s, struct fsmcase *cs)
{
	return fsm_simulate(s, cs->input) == cs->shouldaccept;
}

void run_cases(struct fsmcase cases[], int len, struct fsm *s)
{
	for (int i = 0; i < len; i++) {
		if (!runfsmcase(s, &cases[i])) {
			fprintf(stderr, "\"%s\" case failed\n", cases[i].input);
			exit(1);
		}
	}
}

void
simple_expressions()
{
	struct fsm *s = lex_fsm_fromstring("ab", NULL);
	struct fsmcase cases[] = {
		{false,	""},
		{false,	"a"},
		{false,	"b"},
		{false,	"aa"},
		{true,	"ab"},
	};
	run_cases(cases, LEN(cases), s);
	fsm_destroy(s);

	s = lex_fsm_fromstring("a|b|cd", NULL);
	struct fsmcase cases2[] = {
		{false,	""},
		{true,	"a"},
		{true,	"b"},
		{false, "aa"},
		{false, "ab"},
		{false, "ba"},
		{false, "bb"},
		{true,	"cd"},
	};
	run_cases(cases2, LEN(cases2), s);
	fsm_destroy(s);

	s = lex_fsm_fromstring("a?b?", NULL);
	struct fsmcase cases3[] = {
		{true,	""},
		{true,	"a"},
		{true,	"b"},
		{true,	"ab"},
		{false,	"ba"},
	};
	run_cases(cases3, LEN(cases3), s);
	fsm_destroy(s);

	s = lex_fsm_fromstring("a*b", NULL);
	struct fsmcase cases4[] = {
		{false,	""},
		{false,	"a"},
		{false,	"aa"},
		{true,	"b"},
		{true,	"ab"},
		{true,	"aaab"},
		{false,	"bc"},
		{false,	"abc"},
		{false,	"aabc"},
	};
	run_cases(cases4, LEN(cases4), s);
	fsm_destroy(s);

	s = lex_fsm_fromstring("a[bc0-3]*d", NULL);
	struct fsmcase cases5[] = {
		{false,	""},
		{false,	"a"},
		{false,	"b"},
		{false,	"c"},
		{false,	"d"},
		{false,	"ab"},
		{false,	"ac"},
		{true,	"ad"},
		{true,	"abcccd"},
		{true,	"ab01cd"},
	};
	run_cases(cases5, LEN(cases5), s);
	fsm_destroy(s);
}

void
second_tier()
{
	struct fsm *s = lex_fsm_fromstring("a[bcg-z0-3]*d", NULL);
	struct fsmcase cases[] = {
		{false, "hello, world!"},
		{true,  "ad"},
		{true,  "abd"},
		{true,  "acd"},
		{true,  "abcccccccbbcd"},
		{false, "abcefd"},
		{true,  "abcghijpqrwzzcd"},
		{false, "abcghi9jpqrwzzcd"},
		{true,	"abcghi123jpqrwzzcd"},
	};
	run_cases(cases, LEN(cases), s);
	fsm_destroy(s);
}

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
		struct fsm *s = lex_fsm_fromstring(patterns[i].regex, list);
		list = fsmlist_append(list, dynamic_name(patterns[i].name), s);
	}
	assert(list != NULL);
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
	fsmlist_destroy(list);
}

void
lists()
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
		struct fsm *s = lex_fsm_fromstring(patterns[i].regex, list);
		list = fsmlist_append(list, dynamic_name(patterns[i].name), s);
	}
	fsmlist_destroy(list);
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		simple_expressions,
		second_tier,
		piglatin,
		lists,
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}

}
