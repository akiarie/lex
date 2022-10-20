#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "thompson.h"

struct fsm*
automata_string_conv(char *input)
{
	struct tnode *t = thompson_parse(input);
	tnode_print(t, 0);
	fprintf(stderr, "automata_string_conv not implemented\n");
	exit(1);
	tnode_destroy(t);
}

int
fsm_print(struct fsm *s, int indent)
{
	fprintf(stderr, "fsm_print not implemented\n");
	exit(1);
}
