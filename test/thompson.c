#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "../thompson.h"

struct t_thompson_case {
	char *input;
	char *output;
};

void
confirm_correct(struct t_thompson_case *cs)
{
		struct tnode *n = thompson_parse(cs->input);
		if (strcmp(cs->output, n->output) != 0) {
			fprintf(stderr, "input '%s' got '%s' instead of '%s'\n",
				cs->input, n->output, cs->output);
			exit(1);
		}
		tnode_destroy(n);
}

#define LEN 6

int
main()
{
	struct t_thompson_case cases[LEN] = {
		"a(b|c)*d",           "a.(b|c)*.d",
		"a(ab|c)*d",          "a.(a.b|c)*.d",
		"ab|cd",              "a.b|c.d",
		"(ab)|(cd)",          "(a.b)|(c.d)",
		"andrew|jackson",     "a.n.d.r.e.w|j.a.c.k.s.o.n",
		"(andrew)|(jackson)", "(a.n.d.r.e.w)|(j.a.c.k.s.o.n)",
	};
	for (int i = 0; i < LEN; i++) {
		confirm_correct(&cases[i]);
	}

}
