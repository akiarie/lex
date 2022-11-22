#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "automata_util.c"

struct edge *
edge_create(struct fsm *dest, char c, bool owner)
{
	struct edge *e = (struct edge *) calloc(1, sizeof(struct edge));
	e->c = c;
	e->dest = dest;
	e->owner = owner;
	return e;
}

static void
edge_destroy(struct edge *e)
{
	if (e->owner) {
		fsm_destroy(e->dest);
	}
	free(e);
}


static struct fsmset *
fsmset_create()
{
	struct fsmset *l = (struct fsmset *) calloc(1, sizeof(struct fsmset));
	assert(NULL == l->arr);
	return l;
}

static void
fsmset_destroy(struct fsmset *l)
{
	free(l);
}

static bool
fsmset_in(struct fsmset *l, struct fsm *s)
{
	for (int i = 0; i < l->len; i++) {
		if (l->arr[i] == s) {
			return true;
		}
	}
	return false;
}

static void
fsmset_add(struct fsmset *l, struct fsm *s)
{
	if (fsmset_in(l, s)) {
		return;
	}
	l->arr = (struct fsm **) realloc(l->arr, sizeof(struct fsm) * ++l->len);
	l->arr[l->len-1] = s;
}

static void
fsmset_addrange(struct fsmset *l, struct fsmset *m)
{
	for (int i = 0; i < m->len; i++) {
		fsmset_add(l, m->arr[i]);
	}
}

static struct fsmset *
fsm_move(struct fsm *, char);

static struct fsmset *
fsmset_move(struct fsmset *l, char c)
{
	struct fsmset *next = fsmset_create();
	for (int i = 0; i < l->len; i++) {
		struct fsm *s = l->arr[i];
		struct fsmset *m = fsm_move(s, c);
		fsmset_addrange(next, m);
	}
	return next;
}

static struct fsmset *
fsmset_epsclosure(struct fsmset *l)
{
	struct fsmset *m = fsmset_move(l, '\0');
	fsmset_addrange(m, l);
	return m;
}

static struct fsm *
fsmset_tofsm(struct fsmset *l)
{
	struct fsm *s = fsm_create(false);
	for (int i = 0; i < l->len; i++) {
		struct fsm *t = l->arr[i];
		s->accepting |= t->accepting;
		fsm_addedge(s, edge_create(t, '\0', false));
	}
	return s;
}

struct fsm *
fsm_create(bool accepting)
{
	struct fsm *s = (struct fsm *) calloc(1, sizeof(struct fsm));
	assert(NULL == s->edges);
	s->accepting = accepting;
	return s;
}

void
fsm_destroy(struct fsm *s)
{
	for (int i = 0; i < s->nedges; i++) {
		edge_destroy(s->edges[i]);
	}
	free(s);
}

void
fsm_addedge(struct fsm *s, struct edge *e)
{
	s->edges = (struct edge **)
		realloc(s->edges, sizeof(struct edge) * ++s->nedges);
	s->edges[s->nedges-1] = e;
	s->accepting |= (e->c == '\0') && e->dest->accepting;
}

static struct fsmset *
fsmset_singleton(struct fsm *s)
{
	struct fsmset *l = fsmset_create();
	fsmset_add(l, s);
	return l;
}

static struct fsmset *
fsm_move_act(struct fsm *, char, struct circuitbreaker *);

static struct fsmset *
fsm_reach(struct fsm *s, char c, struct circuitbreaker *tr)
{
	struct circuitbreaker *trnew = circuitbreaker_copy(tr);
	struct fsmset *l = fsm_move_act(s, c, trnew);
	circuitbreaker_destroy(trnew);
	return l;
}

static struct fsmset *
fsm_move_act(struct fsm *s, char c, struct circuitbreaker *tr)
{
	struct fsmset *l = fsmset_create();
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		if (e->c == c) {
			fsmset_add(l, e->dest);
		}
		if (e->c == '\0') {
			fsmset_addrange(l, fsm_reach(e->dest, c, tr));
		}
	}
	return l;
}

static struct fsmset *
fsm_move(struct fsm *s, char c)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmset *l = fsm_move_act(s, c, tr);
	circuitbreaker_destroy(tr);
	return l;
}

struct fsm *
fsm_sim(struct fsm *s, char c)
{
	struct fsmset *l = fsmset_singleton(s);
	struct fsmset *cls = fsmset_epsclosure(l);
	struct fsmset *m = fsmset_move(cls, c);
	return fsmset_tofsm(m);
}

static struct fsmlist*
fsmlist_create(char *name, struct fsm *s)
{
	struct fsmlist *l = (struct fsmlist *) malloc(sizeof(struct fsmlist));
	l->name = name;
	l->s = s;
	l->next = NULL;
	return l;
}

static struct fsmlist*
fsmlist_tail(struct fsmlist *l)
{
	for (; l->next != NULL; l = l->next) {}
	return l;
}


struct fsmlist *
fsmlist_append(struct fsmlist *l, char *name, struct fsm *s)
{
	if (l == NULL) {
		return fsmlist_create(name, s);
	}
	fsmlist_tail(l)->next = fsmlist_create(name, s);
	return l;
}

void
fsmlist_destroy(struct fsmlist *l)
{
	if (l == NULL) {
		return;
	}
	if (l->next != NULL) {
		fsmlist_destroy(l->next);
	}
	if (l->name != NULL) {
		free(l->name);
	}
	free(l);
}

struct fsmlist *
fsmlist_copy(struct fsmlist *l)
{
	fprintf(stderr, "fsmlist_copy NOT IMPLEMENTED\n");
	exit(1);
}

struct findresult *
fsmlist_findnext(struct fsmlist *l, char *input)
{
	fprintf(stderr, "fsmlist_findnext NOT IMPLEMENTED\n");
	exit(1);
}
