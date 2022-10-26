#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"
#include "thompson.h"
#include "util_gen.c"

struct fsm*
util_fsm_fromtree(struct tnode* tree, struct fsmlist *l)
{
	char *typename;
	struct fsm *start, *final;
	struct tnode* copy;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return util_fsm_fromtree(tree->left, l);
		}
		return automata_union(util_fsm_fromtree(tree->left, l),
			util_fsm_fromtree(tree->right, l));

	case NT_CONCAT: case NT_REST:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return util_fsm_fromtree(tree->left, l);
		}
		return automata_concat(util_fsm_fromtree(tree->left, l),
			util_fsm_fromtree(tree->right, l));

	case NT_CLOSED_BLANK:
		return util_fsm_fromtree(tree->left, l);

	case NT_CLOSURE:
		return automata_closure(util_fsm_fromtree(tree->left, l),
			tree->value[0]);

	case NT_BASIC_EXPR:
		copy = tnode_copy(tree);
		copy->type = NT_EXPR;
		return util_fsm_fromtree(copy, l);

	case NT_BASIC_CLASS:
		return automata_class(tree->value);

	case NT_BASIC_ID:
		return automata_id(tree->value, l);

	case NT_SYMBOL:
		start = fsm_create(false);
		fsm_addedge(start, edge_create(fsm_create(true), tree->value[0],
			true));
		return start;

	default:
		typename = tnode_type_string(tree->type);
		fprintf(stderr, "unknown type %s\n", typename);
		free(typename);
		exit(1);
	}
}


struct fsm*
util_fsm_fromstring(char *input, struct fsmlist *l)
{
	struct tnode *t = thompson_parse(input);
	struct fsm *s = util_fsm_fromtree(t, l);
	tnode_destroy(t);
	return s;
}

int
util_numlen(int num)
{
	if (num == 0) {
		return 1;
	}
	int len = 1;
	for (int k = num; k > 0; k /= 10) {
		len++;
	}
	return len;
}

char *
util_name(char *type, int num)
{
	num = 124112;
	int len = strlen(type) + util_numlen(num) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s%d", type, num);
	return name;
}

char *
util_gen_automaton(struct fsm *s, int num)
{
	char *name = util_name("fsm", num);
	printf("struct fsm %s {\n", name);
	printf("\t.accepting = %s,\n", s->accepting ? "true" : "false");
	printf("\t.nedges = %d,\n", s->nedges);
	printf("};\n");
	return name;
}

void
util_gen_driver(char *name)
{
	printf("/* driver code based on %s */\n", name);
}

void
util_gen(struct fsmlist *l, char *varname, FILE *out)
{
	printf("\n/* BEGIN */\n\n");
	char *s0 = util_gen_automaton(l->s, 0);
	printf("struct fsm *%s = %s;\n", varname, s0);
	printf("\n");
	util_gen_driver(s0);
	printf("\n");
	printf("/* fsm_destroy(%s); */\n", varname);
	printf("\n/* END */\n");
}
