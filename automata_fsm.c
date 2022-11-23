#include<stdlib.h>
#include<assert.h>

#include "automata.h"
#include "automata_util.c"

/* edge */

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


/* fsmset */

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
	if (l->len == 0) {
		return NULL;
	}
	struct fsm *s = fsm_create(false);
	for (int i = 0; i < l->len; i++) {
		struct fsm *t = l->arr[i];
		s->accepting |= t->accepting;
		fsm_addedge(s, edge_create(t, '\0', false));
	}
	return s;
}


/* fsm */

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

static struct fsmset *
fsmset_singleton(struct fsm *s)
{
	struct fsmset *l = fsmset_create();
	fsmset_add(l, s);
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

static struct fsm *
fsm_copy_act(struct fsm *s, struct copymap *map)
{
	assert(s != NULL && map != NULL);
	struct fsm *prev = copymap_get(map, s);
	if (prev != NULL) {
		return prev;
	}
	struct fsm *new = fsm_create(s->accepting);
	copymap_append(map, s, new);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		fsm_addedge(new, edge_create(fsm_copy_act(e->dest, map),
			e->c, e->owner));
	}
	return new;
}

static struct fsm *
fsm_copy(struct fsm *s)
{
	struct copymap *map = copymap_create();
	struct fsm *new = fsm_copy_act(s, map);
	copymap_destroy(map);
	return new;
}


/* fsmlist */

static char *
dynamic_name(char *static_name)
{
	assert(static_name != NULL);
	int len = strlen(static_name) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s", static_name);
	return name;
}

static struct fsmlist*
fsmlist_create(char *name, struct fsm *s)
{
	struct fsmlist *l = (struct fsmlist *) calloc(1, sizeof(struct fsmlist));
	assert(l->next == NULL);
	l->name = dynamic_name(name);
	l->s = s;
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
	assert(l->s != NULL);
	fsm_destroy(l->s);
	free(l);
}

static struct fsmlist *
fsmlist_copy(struct fsmlist *l)
{
	struct fsmlist *m = fsmlist_create(
		dynamic_name(l->name), fsm_copy(l->s));
	if (l->next != NULL) {
		m->next = fsmlist_copy(l->next);
	}
	return m;
}

static struct fsmlist *
fsmlist_sim(struct fsmlist *l, char c)
{
	struct fsmlist *m = fsmlist_copy(l);
	struct fsmlist *result = NULL;
	for (struct fsmlist *n = m; n != NULL; n = n->next) {
		struct fsm *s = fsm_sim(n->s, c);
		if (s != NULL) {
			result = fsmlist_append(result, n->name, s);
		}
	}
	fsmlist_destroy(m);
	return result;
}

static struct fsmlist *
fsmlist_firstacc(struct fsmlist *l)
{
	for (; l != NULL && !l->s->accepting; l = l->next) {}
	return l;
}

static struct findresult *
findresult_create(char *fsm, int len)
{
	struct findresult *r = (struct findresult *)
		calloc(1, sizeof(struct findresult));
	if (fsm != NULL) {
		r->fsm = dynamic_name(fsm);
	}
	r->len = len;
	return r;
}

void
findresult_destroy(struct findresult *r)
{
	if (r->fsm != NULL) {
		free(r->fsm);
	}
	free(r);
}

struct findresult *
fsmlist_findnext(struct fsmlist *l, char *input)
{
	assert(l != NULL && input != '\0');
	struct fsmlist *m = fsmlist_sim(l, input[0]);
	if (m == NULL) {
		/* no match */
		return findresult_create(NULL, 1);
	}
	struct fsmlist *n = fsmlist_firstacc(m);
	if (input[1] == '\0') {
		if (n == NULL) {
			fsmlist_destroy(m);
			/* no match */
			return findresult_create(NULL, 1);
		}
		char *fsm = dynamic_name(n->name);
		fsmlist_destroy(m);
		return findresult_create(fsm, 1);
	}
	struct findresult *r = fsmlist_findnext(m, input + 1);
	if (r->fsm != NULL) {
		r->len++;
		fsmlist_destroy(m);
		return r;
	}
	if (n == NULL) {
		fsmlist_destroy(m);
		/* no match */
		return findresult_create(NULL, 1);
	}
	char *fsm = dynamic_name(n->name);
	fsmlist_destroy(m);
	return findresult_create(fsm, 1);
}
