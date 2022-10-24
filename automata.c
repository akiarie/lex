#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "thompson.h"

/* r = s·t */
struct fsm*
automata_concat(struct fsm *s, struct fsm *t)
{
	assert(s != NULL && t != NULL);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e->dest != NULL);
		if (e->dest->accepting) {
			fsm_addedge(e->dest, edge_create(t, '\0'));
		} else { // for now only concat unaccepting
			automata_concat(e->dest, t);
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

/* r = s* | s+ | s? */
struct fsm*
automata_closure(struct fsm *s, char c)
{
	struct fsm *closure;
	switch (c) {
	case '?':
		return automata_union(fsm_create(true), s);
	case '*':
		closure = automata_closure(s, '?');
		closure = automata_concat(closure, closure);
		return closure;
	case '+':
		return automata_concat(s, automata_closure(s, '*'));
	default:
		fprintf(stderr, "'%c' is not a closure symbol\n", c);
		exit(1);
	}
}

struct fsm*
automata_tree_conv(struct tnode* tree)
{
	char *typename;
	struct fsm *start, *final;
	struct tnode* copy;
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

	case NT_CLOSURE:
		return automata_closure(automata_tree_conv(tree->left),
			tree->value[0]);

	case NT_BASIC_EXPR:
		copy = tnode_copy(tree);
		copy->type = NT_EXPR;
		return automata_tree_conv(copy);

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

struct sctracker {
	struct fsm *s;
	struct sctracker *next;
};

struct sctracker*
sctracker_create(struct fsm *s)
{
	struct sctracker *tr = (struct sctracker *)
		malloc(sizeof(struct sctracker));
	tr->s = s;
	tr->next = NULL;
	return tr;
}

void
sctracker_destroy(struct sctracker *this)
{
	assert(this != NULL);
	if (this->next != NULL) {
		sctracker_destroy(this->next);
	}
	free(this);
}

bool
sctracker_append(struct sctracker *this, struct fsm *s)
{
	struct sctracker *tr, *prev;
	for (tr = this; tr != NULL; prev = tr, tr = tr->next) {
		if (tr->s == s) {
			return false;
		}
	}
	prev->next = sctracker_create(s);
	return true;
}


struct fsm*
fsm_act_sim(struct fsm *s, struct sctracker *tr, char c);

struct fsm*
edge_traverse(struct edge *e, struct sctracker *tr, char c)
{
	assert(e->dest != NULL);
	if (e->c == c){
		return e->dest;
	} else if (e->c == '\0' && sctracker_append(tr, e->dest)) {
		return fsm_act_sim(e->dest, tr, c);
	}
	return NULL;
}


struct fsm*
fsm_act_sim(struct fsm *s, struct sctracker *tr, char c)
{
	assert(s != NULL);
	struct fsm *S = fsm_create(false);
	for (int i = 0; i < s->nedges; i++) {
		struct fsm *next = edge_traverse(s->edges[i], tr, c);
		if (next != NULL) {
			fsm_addedge(S, edge_create(next, '\0'));
		}
	}
	return (S->nedges > 0) ? S : NULL;
}

struct fsm*
fsm_sim(struct fsm *s, char c)
{
	struct sctracker *tr = sctracker_create(s);
	struct fsm *next = fsm_act_sim(s, tr, c);
	sctracker_destroy(tr);
	return next;
}

bool
fsm_isaccepting(struct fsm *s)
{
	if (s == NULL) {
		return false;
	}
	if (s->accepting) {
		return true;
	}
	for (int i = 0; i < s->nedges; i++) {
		struct sctracker *tr = sctracker_create(s);
		struct fsm *N = edge_traverse(s->edges[i], tr, '\0');
		sctracker_destroy(tr);
		if (fsm_isaccepting(N)) {
			return true;
		}
	}
	return false;
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
fsm_print_act(struct fsm *s, int level, int thisnum)
{
	assert(s != NULL);
	int num = thisnum;
	automata_indent(level);
	if (s->accepting) {
		printf("[%d, Y](%d)\n", num, s->nedges);
	} else {
		printf("[%d](%d)\n", num, s->nedges);
	}
	if (s->nedges > 0) {
		automata_indent(level);
		printf("|\n");
	}
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		automata_indent(level);
		if (s->accepting) {
			printf("-- %c -->\n", e->c);
		} else {
			printf("-- %c -->\n", e->c);
		}
		num += fsm_print_act(e->dest, level + 1, num + 1);
	}
	return s->nedges;
}

int
fsm_print(struct fsm *s)
{
	return fsm_print_act(s, 0, 0);
}
