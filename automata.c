#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "thompson.h"

struct edge*
edge_create(struct fsm *state, char c)
{
	assert(state != NULL);
	struct edge *t = (struct edge *) malloc(sizeof(struct edge));
	t->c = c;
	t->state = state;
	return t;
}

void
automata_final_point(struct fsm *s, struct fsm *to, bool oldremain)
{
	assert(to != NULL);
	for (int i = 0; i < s->nedges; i++) {
		struct fsm *st = s->edges[i]->state;
		if (st == NULL || !st->accepting) {
			continue;
		}
		st->accepting = oldremain;
		fsm_addedge(st, edge_create(to, '\0'));
	}
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	assert(s != NULL && t != NULL);
	struct fsm *final = fsm_create(true);
	automata_final_point(s, final, false);
	automata_final_point(t, final, false);
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0'));
	fsm_addedge(start, edge_create(t, '\0'));
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
		return automata_union(fsm_create(true), s);
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
		start = fsm_create(false);
		fsm_addedge(start, edge_create(fsm_create(true), tree->value[0]));
		return start;

	case NT_EMPTY:
		start = fsm_create(false);
		fsm_addedge(start, edge_create(fsm_create(true), '\0'));
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
fsm_create(bool accepting)
{
	struct fsm *s = (struct fsm *) malloc(sizeof(struct fsm));
	s->accepting = accepting;
	s->nedges = 0;
	s->edges = NULL;
	return s;
}

void
fsm_destroy(struct fsm *s)
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
fsm_addedge(struct fsm *s, struct edge *e)
{
	assert(e->state != NULL);
	s->edges = (struct edge **) realloc(s->edges,
		sizeof(struct edge) * (++s->nedges));
	s->edges[s->nedges-1] = e;
	printf("addedge on '%c' to state with accepting == %d\n", e->c,
		e->state->accepting);
}

struct fsm*
fsm_sim(struct fsm *s, char c)
{
	assert(s != NULL);
	if (s->nedges == 0) {
		return NULL;
	}

	// |- s is actually ε-closure(s)
	struct fsm *S = fsm_create(false);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e->state != NULL);
		if (e->c == '\0') {
			struct fsm *T = fsm_sim(e->state, c);
			if (T == NULL) {
				continue;
			}
			fsm_addedge(S, edge_create(T, '\0'));
		} else if (e->c == c) {
			fsm_addedge(S, edge_create(e->state, '\0'));
		}
	}
	return S;
}

bool
fsm_isaccepting(struct fsm *s)
{
	if (s == NULL) {
		return false;
	}
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		if (e->c == '\0' && fsm_isaccepting(e->state)) {
			return true;
		}
	}
	return s->accepting;
}
