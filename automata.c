#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "thompson.h"

/* r = s·t */
struct fsm*
automata_concat(struct fsm *s, struct fsm *t)
{
	assert(!s->accepting);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e->dest != NULL);
		if (e->dest->accepting) {
			// FIXME: free e->dest
			e->dest = t;
		}
	}
	return s;
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0'));
	fsm_addedge(start, edge_create(t, '\0'));
	struct fsm *final = fsm_create(true);
	automata_concat(s, final);
	automata_concat(t, final);
	return start;
}

struct fsm*
automata_tree_conv(struct tnode* tree)
{
	char *typename;
	struct fsm *start, *final;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return automata_tree_conv(tree->left);
		}
		return automata_union(automata_tree_conv(tree->left),
			automata_tree_conv(tree->right));

	case NT_CONCAT: case NT_REST:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return automata_tree_conv(tree->left);
		}
		return automata_concat(automata_tree_conv(tree->left),
			automata_tree_conv(tree->right));

	case NT_CLOSED_BLANK:
		return automata_tree_conv(tree->left);

	case NT_SYMBOL:
		start = fsm_create(false);
		fsm_addedge(start, edge_create(fsm_create(true), tree->value[0]));
		return start;

	default:
		typename = tnode_type_string(tree->type);
		fprintf(stderr, "unknown type %s\n", typename);
		free(typename);
		exit(1);
	}
}


struct fsm*
automata_string_conv(char *input)
{
	struct tnode *t = thompson_parse(input);
	tnode_print(t, 0);
	struct fsm *nfa = automata_tree_conv(t);
	tnode_destroy(t);
	return nfa;
}


struct edge*
edge_create(struct fsm *dest, char c)
{
	assert(dest != NULL);
	struct edge *t = (struct edge *) malloc(sizeof(struct edge));
	t->c = c;
	t->dest = dest;
	return t;
}


struct fsm*
edge_traverse(struct edge *e, char c)
{
	assert(e->dest != NULL);
	return (e->c == c) ? e->dest : fsm_sim(e->dest, c);
}


struct fsm*
fsm_sim(struct fsm *s, char c)
{
	assert(s != NULL);
	struct fsm *S = fsm_create(false);
	for (int i = 0; i < s->nedges; i++) {
		struct fsm *next = edge_traverse(s->edges[i], c);
		if (next != NULL) {
			fsm_addedge(S, edge_create(next, '\0'));
		}
	}
	return (S->nedges > 0) ? S : NULL;
}

void
fsm_addedge(struct fsm *s, struct edge *e)
{
	assert(e->dest != NULL);
	s->edges = (struct edge **) realloc(s->edges,
		sizeof(struct edge) * (++s->nedges));
	s->edges[s->nedges-1] = e;
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
	fprintf(stderr, "fsm_destroy not implemented\n");
	exit(1);
}

void
automata_indent(int len)
{
	for (int j = 0; j < len; j++) {
		printf("\t");
	}
}

int
fsm_print(struct fsm *s, int base)
{
	assert(s != NULL);
	int num = base;
	automata_indent(base);
	if (s->accepting) {
		printf("[%d, Y](%d)\n", base, s->nedges);
	} else {
		printf("[%d](%d)\n", base, s->nedges);
	}
	if (s->nedges > 0) {
		automata_indent(base);
		printf("|\n");
	}
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		automata_indent(base);
		if (s->accepting) {
			printf("--(%c)-->\n", e->c);
		} else {
			printf("--(%c)-->\n", e->c);
		}
		num += fsm_print(e->dest, num + 1);
	}
	return s->nedges;
}
