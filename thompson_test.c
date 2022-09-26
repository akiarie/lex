#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "thompson.h"

struct testcase {
	char *input;
	char *output;
};

void
run(struct testcase *cs)
{
		struct tnode *n = thompson_parse(cs->input);
		if (strcmp(cs->output, n->output) != 0) {
			fprintf(stderr, "input '%s' got '%s' instead of '%s'\n",
				cs->input, n->output, cs->output);
			exit(1);
		}
		tnode_destroy(n);
}

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

int
main()
{
	struct testcase cases[] = {
		{"a(b|c)*d",		"a.(b|c)*.d"},
		{"a(ab|c)*d",		"a.(a.b|c)*.d"},
		{"ab|cd",		"a.b|c.d"},
		{"(ab)|(cd)",		"(a.b)|(c.d)"},
		{"andrew|jackson",	"a.n.d.r.e.w|j.a.c.k.s.o.n"},
		{"(andrew)|(jackson)",	"(a.n.d.r.e.w)|(j.a.c.k.s.o.n)"},
		{"a[bcd]efg",		"a.[bcd].e.f.g"},
		{"a[b-z0-9abc]efg",	"a.[b-z0-9abc].e.f.g"},
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		run(&cases[i]);
	}
}
