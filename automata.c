#include<stdio.h>
#include<stdlib.h>

#include "automata.h"
#include "thompson.h"

struct edge*
edge_create(char *symbol, struct fsm *states[])
{
	struct edge *t
		= (struct edge *) malloc(sizeof(struct edge));
	t->symbol = symbol;
	*t->states = states;
	return t;
}

struct fsm*
automata_closure(struct fsm *basic, char closure)
{
		fprintf(stderr, "CLOSURE NOT IMPLEMENTED");
		exit(1);
}

struct fsm*
automata_fromclass(struct tnode* class)
{
		fprintf(stderr, "CLASS NOT IMPLEMENTED");
		exit(1);
}

struct fsm*
automata_tree_conv(struct tnode* tree)
{
	struct fsm *next, **states;
	struct edge *t, **edges;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		return automata_or(automata_tree_conv(tree->left),
			automata_tree_conv(tree->right));

	case NT_CONCAT: case NT_REST:
		return automata_concat(automata_tree_conv(tree->left),
			automata_tree_conv(tree->right));

	case NT_CLOSED_BLANK:
		return automata_tree_conv(tree->left);

	case NT_CLOSURE:
		return automata_closure(automata_tree_conv(tree->left),
			tree->value[0]);

	case NT_BASIC_EXPR:
		return automata_tree_conv(tree->left);

	case NT_BASIC_CLASS:
		return automata_fromclass(tree);

	case NT_BASIC_ID:
		// FIXME: invoke FSM corresponding to ID
		fprintf(stderr, "ID NOT IMPLEMENTED");
		exit(1);

	case NT_SYMBOL:
		states = (struct fsm **) malloc(sizeof(struct fsm));
		states[0] = fsm_create(true, NULL);
		edges = (struct edge **) malloc(sizeof(struct edge));
		edges[0] = edge_create(tree->value, states);
		return fsm_create(false, edges);

	case NT_EMPTY:
		states = (struct fsm **) malloc(sizeof(struct fsm));
		states[0] = fsm_create(true, NULL);
		edges = (struct edge **) malloc(sizeof(struct edge));
		edges[0] = edge_create("", states);
		return fsm_create(false, edges);
	default:
		fprintf(stderr, "NOT IMPLEMENTED\n");
		exit(1);
	}
}

struct fsm*
automata_string_conv(char *input)
{
	struct tnode* tree = thompson_parse(input);
	return automata_tree_conv(tree);
}

struct fsm*
fsm_create(bool accepting, struct edge *edges[])
{
	struct fsm *s = (struct fsm *) malloc(sizeof(struct fsm));
	s->accepting = accepting;
	*s->edges = edges;
	return s;
}
