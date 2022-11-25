#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<assert.h>

#include "thompson.h"
#include "automata.h"
#include "lex.h"

static struct fsm *
fsm_fromtree(struct tnode* tree, struct fsmlist *l)
{
	char *typename;
	struct fsm *start, *final;
	struct tnode* copy;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return fsm_fromtree(tree->left, l);
		}
		return automata_union(fsm_fromtree(tree->left, l),
			fsm_fromtree(tree->right, l));

	case NT_CONCAT: case NT_REST:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return fsm_fromtree(tree->left, l);
		}
		start = fsm_fromtree(tree->left, l);
		automata_concat(start, fsm_fromtree(tree->right, l), true);
		return start;

	case NT_CLOSED_BLANK:
		return fsm_fromtree(tree->left, l);

	case NT_CLOSURE:
		return automata_closure(fsm_fromtree(tree->left, l),
			tree->value[0]);

	case NT_BASIC_EXPR:
		copy = tnode_copy(tree);
		copy->type = NT_EXPR;
		return fsm_fromtree(copy, l);

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

struct fsm *
lex_fsm_fromstring(char *input, struct fsmlist *l)
{
	struct tnode *t = thompson_parse(input);
	struct fsm *s = fsm_fromtree(t, l);
	tnode_destroy(t);
	return s;
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

struct lexer {
	char *pre, *post;
};

struct lexer *
lexer_create(struct lexfile *f)
{
	struct lexer *lx = (struct lexer *) malloc(sizeof(struct lexer));
	return lx;
}

void
lexer_destroy(struct lexer *lx)
{
	assert(lx != NULL);
	free(lx->pre);
	free(lx->post);
	free(lx);
}
