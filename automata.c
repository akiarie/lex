#include<stdio.h>
#include<stdlib.h>

#include "automata.h"
#include "thompson.h"

struct edge*
edge_create(struct fsm *state, char *symbol)
{
	struct edge *t = (struct edge *) malloc(sizeof(struct edge));
	t->symbol = symbol;
	t->state = state;
	return t;
}

void
automata_final_point(struct fsm *s, struct fsm *to, bool oldremain)
{
	for (int i = 0; i < s->nedges; i++) {
		struct fsm *st = s->edges[i]->state;
		if (st == NULL || !st->accepting) {
			continue;
		}
		st->accepting = oldremain;
		int n = st->nedges;
		fsm_realloc(st, n + 1);
		st->edges[n] = edge_create(to, "");
	}
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	struct fsm *final = fsm_create(true, 0);
	automata_final_point(s, final, false);
	automata_final_point(t, final, false);
	struct fsm *start = fsm_create(false, 2);
	start->edges[0] = edge_create(s, "");
	start->edges[1] = edge_create(t, "");
	return start;
}

/* r = s·t */
struct fsm*
automata_concat(struct fsm *s, struct fsm *t)
{
	automata_final_point(s, t, false);
	return s;
}

/* r = s* | s+ | s? */
struct fsm*
automata_closure(struct fsm *s, char c)
{
	switch (c) {
	case '*':
		automata_final_point(s, s, true);
		return s;
	case '+':
		return automata_concat(s, automata_closure(s, '*'));
	case '?':
		return automata_union(fsm_create(true, 0), s);
	default:
		fprintf(stderr, "'%c' is not a closure symbol\n", c);
		exit(1);
	}
}

struct fsm*
automata_fromclass(struct tnode* class)
{
	fprintf(stderr, "CLASS NOT IMPLEMENTED\n");
	exit(1);
}

struct fsm*
automata_tree_conv(struct tnode* tree)
{
	struct fsm *start, *final;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		return automata_union(automata_tree_conv(tree->left),
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
		fprintf(stderr, "ID NOT IMPLEMENTED\n");
		exit(1);

	case NT_SYMBOL:
		start = fsm_create(false, 1);
		start->edges[0] = edge_create(fsm_create(true, 0), tree->value);
		return start;

	case NT_EMPTY:
		start = fsm_create(false, 1);
		start->edges[0] = edge_create(fsm_create(true, 0), "");
		return start;
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
fsm_create(bool accepting, int nedges)
{
	struct fsm *s = (struct fsm *) malloc(sizeof(struct fsm));
	s->accepting = accepting;
	s->nedges = nedges;
	s->edges = NULL;
	if (nedges > 0) {
		s->edges = (struct edge **) malloc(sizeof(struct edge) * nedges);
	}
	return s;
}

void fsm_destroy(struct fsm *s)
{
	if (s->edges != NULL) {
		for (int i = 0; i < s->nedges; i++) {
			struct fsm *st = s->edges[i]->state;
			if (st != NULL) {
				// TODO: dispose
			}
		}
		free(s->edges);
	}
	free(s);
}

void
fsm_realloc(struct fsm *s, int nedges)
{
	s->nedges = nedges;
	s->edges = (struct edge **) realloc(s->edges,
		sizeof(struct edge) * nedges);
}
