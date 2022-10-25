#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

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
