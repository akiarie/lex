#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"
#include "automata_fsm.c"

static struct fsmset *
acceptors_act(struct fsm *s, struct circuitbreaker *tr)
{
	struct fsmset *l = fsmset_create();
	if (s->accepting) {
		fsmset_add(l, s);
	}
	struct circuitbreaker *trnew = circuitbreaker_copy(tr);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		/* ε-edges are only allowed if we haven't seen them before */
		if (e->c == '\0' && !circuitbreaker_append(trnew, e->dest)) {
			continue;
		}
		fsmset_addrange(l, acceptors_act(e->dest, trnew));
	}
	circuitbreaker_destroy(trnew);
	return l;
}

static struct fsmset *
acceptors(struct fsm *s)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmset *l = acceptors_act(s, tr);
	circuitbreaker_destroy(tr);
	return l;
}

void
automata_concat(struct fsm *s, struct fsm *t, bool owner)
{
	int nowners = owner ? 0 : 1;
	struct fsmset *l = acceptors(s);
	for (int i = 0; i < l->len; i++) {
		struct fsm *r = l->arr[i];
		r->accepting = false;
		fsm_addedge(r, edge_create(t, '\0', nowners++ == 0));
	}
	fsmset_destroy(l);
}

struct fsm *
automata_union(struct fsm *s, struct fsm *t)
{
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(t, '\0', true));
	struct fsm *final = fsm_create(true);
	automata_concat(s, final, true);
	automata_concat(t, final, false); /* final can only be owned once */
	return start;
}

static struct fsm *
automata_closure_ast(struct fsm *s)
{
	struct fsm *start = fsm_create(false);
	struct fsm *final = fsm_create(true);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(final, '\0', true));
	struct fsmset *l = acceptors(s);
	for (int i = 0; i < l->len; i++) {
		struct fsm *t = l->arr[i];
		t->accepting = false;
		fsm_addedge(t, edge_create(s, '\0', false));
		fsm_addedge(t, edge_create(final, '\0', false));
	}
	return start;
}

/* r = s* | s+ | s? */
struct fsm *
automata_closure(struct fsm *s, char c)
{
	struct fsm *closure;
	switch (c) {
	case '?':
		return automata_union(fsm_create(true), s);
	case '*':
		return automata_closure_ast(s);
	case '+':
		closure = automata_closure(s, '*');
		automata_concat(s, closure, false);
		return closure;
	default:
		fprintf(stderr, "'%c' is not a closure symbol\n", c);
		exit(1);
	}
}

struct classlist {
	char c;
	struct classlist *next;
};

static void
classlist_destroy(struct classlist *l)
{
	if (l->next != NULL) {
		classlist_destroy(l->next);
	}
	free(l);
}

static struct classlist *
classlist_create(char c)
{
	struct classlist *l = (struct classlist *)
		calloc(1, sizeof(struct classlist));
	l->c = c;
	return l;
}

static void
classlist_print(struct classlist *l)
{
	printf("classlist[");
	for (; l != NULL; l = l->next) {
		printf("%c", l->c);
		if (l->next != NULL) {
			printf(", ");
		}
	}
	printf("]\n");
}

static struct classlist *
classlist_fromrange(char a, char b)
{
	assert(a <= b);
	struct classlist *head = classlist_create(a);
	struct classlist *l = head;
	for (char c = a + 1; c != b + 1; c++) {
		l->next = classlist_create(c);
		l = l->next;
	}
	return head;
}

static struct classlist *
classlist_advance(char **sp)
{
	char *s = *sp;
	if (s[1] == '-') {
		assert(s[2] != '\0'); // assume all ranges closed
		*sp += 2;
		return classlist_fromrange(s[0], s[2]);
	}
	return classlist_create(s[0]);
}

static struct classlist *
classlist_tail(struct classlist *l)
{
	for (; l->next != NULL; l = l->next) {}
	return l;
}

static struct classlist *
classlist_fromstring(char *s)
{
	assert(s != '\0');
	struct classlist *head = classlist_advance(&s);
	struct classlist *l = head;
	for (s++; *s != '\0'; s++) {
		classlist_tail(l)->next = classlist_advance(&s);
	}
	return head;
}

struct fsm *
automata_class(char *value)
{
	struct classlist *list = classlist_fromstring(value);
	struct fsm* start = fsm_create(false);
	struct fsm* final = fsm_create(true);
	for (struct classlist *l = list; l != NULL; l = l->next) {
		fsm_addedge(start, edge_create(final, l->c, false));
	}
	classlist_destroy(list);
	return start;
}

struct fsm *
automata_id(char *id, struct fsmlist *l)
{
	for (; l != NULL; l = l->next) {
		if (strcmp(id, l->name) == 0) {
			return fsm_copy(l->s);
		}
	}
	fprintf(stderr, "unknown pattern '%s' has not been declared\n", id);
	exit(1);
}
