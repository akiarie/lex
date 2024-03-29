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
		char *output = tnode_output(n);
		if (strcmp(cs->output, output) != 0) {
			fprintf(stderr, "input '%s' got '%s' instead of '%s'\n",
				cs->input, output, cs->output);
			exit(1);
		}
		free(output);
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
		{"a[bcd]?efg",		"a.[bcd]?.e.f.g"},
		{"a[^b-z0-9abc]+efg",	"a.[^b-z0-9abc]+.e.f.g"},
		{"{robert}+[a-z]*bro",	"{robert}+.[a-z]*.b.r.o"},
		{"ab\\t\\n[ab\\t]cd",	"a.b.\t.\n.[ab\t].c.d"},
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		run(&cases[i]);
	}
}
