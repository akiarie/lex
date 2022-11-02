#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"


/* circuitbreaker: tracker to prevent ε-loops */
struct circuitbreaker {
	void *s;
	struct circuitbreaker *next;
};

int
circuitbreaker_len(struct circuitbreaker *tr)
{
	int n = 0;
	for (struct circuitbreaker *next = tr; tr != NULL; tr = tr->next) {
		n++;
	}
	return n;
}


struct circuitbreaker*
circuitbreaker_create(void *s)
{
	struct circuitbreaker *tr = (struct circuitbreaker *)
		malloc(sizeof(struct circuitbreaker));
	tr->s = s;
	tr->next = NULL;
	return tr;
}


struct circuitbreaker*
circuitbreaker_copy(struct circuitbreaker *tr)
{
	struct circuitbreaker *new = circuitbreaker_create(tr->s);
	if (tr->next != NULL) {
		new->next = circuitbreaker_copy(tr->next);
	}
	return new;
}


void
circuitbreaker_destroy(struct circuitbreaker *tr)
{
	assert(tr != NULL);
	if (tr->next != NULL) {
		circuitbreaker_destroy(tr->next);
	}
	free(tr);
}


bool
circuitbreaker_append(struct circuitbreaker *tr, void *s)
{
	for (; tr->s != s; tr = tr->next) {
		if (tr->next == NULL) {
			tr->next = circuitbreaker_create(s);
			return true;
		}
	}
	return false;
}

struct edge*
edge_create(struct fsm *dest, char c, bool owner)
{
	assert(dest != NULL);
	struct edge *e = (struct edge *) malloc(sizeof(struct edge));
	e->c = c;
	e->dest = dest;
	e->owner = owner;
	return e;
}

void edge_destroy(struct edge *e)
{
	assert(e->dest != NULL);
	if (e->owner) {
		fsm_destroy(e->dest);
	}
	free(e);
}

/* r = s · t */
struct fsm*
automata_concat(struct fsm *s, struct fsm *t, bool owner)
{
	int nowners = owner ? 0 : 1;
	for (struct fsmlist *l = fsm_finals(s); l != NULL; l = l->next) {
		assert(l->s->accepting);
		l->s->accepting = false;
		fsm_addedge(l->s, edge_create(t, '\0', nowners++ == 0));
	}
	return s;
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	assert(s != t);
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(t, '\0', true));
	struct fsm *final = fsm_create(true);
	automata_concat(s, final, true);
	automata_concat(t, final, false);
	return start;
}

struct fsm*
automata_closure_ast(struct fsm *s)
{
	struct fsm *start = fsm_create(false);
	struct fsm *final = fsm_create(true);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(final, '\0', true));
	for (struct fsmlist *l = fsm_finals(s); l != NULL; l = l->next) {
		assert(l->s->accepting);
		l->s->accepting = false;
		fsm_addedge(l->s, edge_create(s, '\0', false));
		fsm_addedge(l->s, edge_create(final, '\0', false));
	}
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
		return automata_closure_ast(s);
	case '+':
		return automata_concat(s, automata_closure(s, '*'), false);
	default:
		fprintf(stderr, "'%c' is not a closure symbol\n", c);
		exit(1);
	}
}

struct classlist {
	char c;
	struct classlist *next;
};

void
classlist_destroy(struct classlist *l)
{
	if (l->next != NULL) {
		classlist_destroy(l->next);
	}
	free(l);
}

struct classlist*
classlist_create(char c)
{
	struct classlist *l = (struct classlist *) malloc(sizeof(struct classlist));
	l->c = c;
	l->next = NULL;
	return l;
}

void
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

struct classlist*
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

struct classlist*
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

struct classlist*
classlist_tail(struct classlist *l)
{
	for (; l->next != NULL; l = l->next) {}
	return l;
}

struct classlist*
classlist_fromstring(char *s)
{
	assert(s != '\0');
	struct classlist *head = classlist_create(s[0]);
	struct classlist *l = head;
	for (s++; *s != '\0'; s++) {
		l->next = classlist_advance(&s);
		l = classlist_tail(l->next);
	}
	return head;
}

struct fsm*
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

struct fsm*
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

struct fsmlist*
fsm_finals_act(struct fsm *s, struct circuitbreaker *tr)
{
	struct fsmlist *l = NULL;
	if (s->accepting) {
		l = fsmlist_append(l, NULL, s);
	}
	struct circuitbreaker *trnew = circuitbreaker_copy(tr);
	for (int i = 0; i < s->nedges; i++) {
		struct edge  *e = s->edges[i];
		assert(e != NULL);
		if (e->c == '\0' && !circuitbreaker_append(trnew, e->dest)) {
			continue;
		}
		for (struct fsmlist *m = fsm_finals_act(e->dest, trnew); m != NULL;
				m = m->next) {
			l = fsmlist_append(l, m->name, m->s);
		}
	}
	circuitbreaker_destroy(trnew);
	return l;
}

struct fsmlist*
fsm_finals(struct fsm *s)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmlist *l = fsm_finals_act(s, tr);
	circuitbreaker_destroy(tr);
	assert(l != NULL);
	return l;
}

struct fsmlist*
fsmlist_fromfsm(struct fsm *s)
{
	return fsmlist_append(NULL, NULL, s);
}

static struct fsmlist*
fsmlist_create(char *name, struct fsm *s);

struct fsmlist*
fsm_epsclosure_act(struct fsm *s, struct circuitbreaker *tr)
{
	struct fsmlist *l = fsmlist_create(NULL, s);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e != NULL);
		if (e->c != '\0' || !circuitbreaker_append(tr, e->dest)) {
			continue;
		}
		for (struct fsmlist *m = fsm_epsclosure_act(e->dest, tr);
				m != NULL; m = m->next) {
			l = fsmlist_append(l, m->name, m->s);
		}
	}
	return l;
}

struct fsmlist*
fsm_epsclosure(struct fsm *s)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmlist *l = fsm_epsclosure_act(s, tr);
	circuitbreaker_destroy(tr);
	return l;
}

struct fsmlist*
fsm_move(struct fsm *s, char c)
{
	assert(c != '\0');
	struct fsmlist *l = NULL;
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e != NULL);
		if (e->c == c) {
			l = fsmlist_append(l, NULL, e->dest);
		}
	}
	return l;
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

static struct fsm*
fsm_copy_act(struct fsm *s, struct circuitbreaker *tr);

struct fsm*
fsm_copy_dest(struct edge *e, struct circuitbreaker *tr)
{
	if (e->c == '\0' && !circuitbreaker_append(tr, e->dest)) {
		return e->dest;
	}
	return fsm_copy_act(e->dest, tr);
}

static struct fsm*
fsm_copy_act(struct fsm *s, struct circuitbreaker *tr)
{
	assert(s != NULL);
	struct fsm *new = fsm_create(s->accepting);
	struct circuitbreaker *trnew = circuitbreaker_copy(tr);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		fsm_addedge(new, edge_create(fsm_copy_dest(e, trnew), e->c,
			e->owner));
	}
	circuitbreaker_destroy(trnew);
	return new;
}

struct fsm*
fsm_copy(struct fsm *s)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsm *new = fsm_copy_act(s, tr);
	circuitbreaker_destroy(tr);
	return new;
}

void
fsm_destroy(struct fsm *s)
{
	assert(s != NULL);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e->dest != NULL);
		edge_destroy(e);
	}
	free(s);
}

void
automata_indent(int len)
{
	for (int j = 0; j < len; j++) {
		printf("\t");
	}
}

struct fsmcounter {
	void *p;
	struct fsmcounter *next;
};

struct fsmcounter*
fsmcounter_create(void *p)
{
	struct fsmcounter *c = (struct fsmcounter *)
		malloc(sizeof(struct fsmcounter));
	c->p = p;
	c->next = NULL;
	return c;
}

void
fsmcounter_destroy(struct fsmcounter *c)
{
	if (c->next != NULL) {
		fsmcounter_destroy(c->next);
	}
	free(c);
}

int
fsmcounter_count(struct fsmcounter *c, void *p)
{
	assert(c != NULL);
	if (c->p == p) {
		// repeat
		return 0;
	}
	if (c->next == NULL) {
		c->next = fsmcounter_create(p);
	}
	return 1 + fsmcounter_count(c->next, p);
}


static void
fsm_print_edge_indent(int level)
{
	automata_indent(level);
	printf("|\n");
	automata_indent(level);
}

static void
fsm_print_node(struct fsm *s, int level, int count)
{
	automata_indent(level);
	if (s->accepting) {
		printf("[%d, acc](%d)\n", count, s->nedges);
	} else {
		printf("[%d](%d)\n", count, s->nedges);
	}
}


int
fsm_print_act(struct fsm *s, int level, struct fsmcounter *cnt,
		struct circuitbreaker *tr);

static int
fsm_print_edge(struct edge *e, int level, struct fsmcounter *cnt, struct
		circuitbreaker *tr)
{
	int num = 0;
	if (circuitbreaker_append(tr, e->dest)) {
		printf("\n");
		num = fsm_print_act(e->dest, level, cnt, tr);
	} else {
		printf("*\n");
	}
	return num;
}

static char*
properc(char c)
{
	int len;
	char *n;
	if (c == '\0') {
		len = strlen("ε") + 1;
		n = (char *) malloc(sizeof(char) * len);
		snprintf(n, len, "ε");
	} else {
		len = 2;
		n = (char *) malloc(sizeof(char) * len);
		snprintf(n, len, "%c", c);
	}
	return n;
}

int
fsm_print_act(struct fsm *s, int level, struct fsmcounter *cnt,
		struct circuitbreaker *tr)
{
	assert(s != NULL && cnt != NULL && tr != NULL);
	int num = fsmcounter_count(cnt, s);
	fsm_print_node(s, level, num);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		fsm_print_edge_indent(level);
		char *proper = properc(e->c);
		printf("-- %s -%s>", proper, e->owner ? ":" : "-");
		free(proper);
		struct circuitbreaker *trnew = circuitbreaker_copy(tr);
		num += fsm_print_edge(e, level + 1, cnt, trnew);
		circuitbreaker_destroy(trnew);
	}
	return s->nedges;
}

void
fsm_print(struct fsm *s)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmcounter *cnt = fsmcounter_create(s);
	fsm_print_act(s, 0, cnt, tr);
	fsmcounter_destroy(cnt);
	circuitbreaker_destroy(tr);
}

struct fsmlist*
fsmlist_epsclosure(struct fsmlist *l)
{
	if (l == NULL) {
		return NULL;
	}
	assert(l->s != NULL);
	struct fsmlist *m = fsm_epsclosure(l->s);
	for (struct fsmlist *n = fsmlist_epsclosure(l->next); n != NULL;
			n = n->next) {
		m = fsmlist_append(m, n->name, n->s);
	}
	return m;
}

struct fsmlist*
fsmlist_move(struct fsmlist *l, char c)
{
	if (l == NULL) {
		return NULL;
	}
	assert(l->s != NULL);
	struct fsmlist *m = fsm_move(l->s, c);
	for (struct fsmlist *n = fsmlist_move(l->next, c); n != NULL;
			n = n->next) {
		m = fsmlist_append(m, n->name, n->s);
	}
	return m;
}

struct fsmlist*
fsmlist_sim(struct fsmlist *l, char c)
{
	return fsmlist_epsclosure(fsmlist_move(l, c));
}


bool
fsmlist_accepting(struct fsmlist *l)
{
	for (; l != NULL; l = l->next) {
		assert(l->s != NULL);
		if (l->s->accepting) {
			return true;
		}
	}
	return false;
}

struct fsmlist*
fsmlist_tail(struct fsmlist *l)
{
	for (; l->next != NULL; l = l->next) {}
	return l;
}

void
fsmlist_print_act(struct fsmlist *l, int level, struct fsmcounter *cnt)
{
	automata_indent(level);
	if (l == NULL) {
		printf("NULL\n");
		return;
	}
	assert(l != NULL && cnt != NULL);
	int thislevel = level + 1;

	printf("fsmlist[\n");

	automata_indent(thislevel);
	printf("'%s':\n", l->name == NULL ? "" : l->name);
	struct circuitbreaker *tr = circuitbreaker_create(l->s);
	fsm_print_act(l->s, thislevel, cnt, tr);
	circuitbreaker_destroy(tr);
	automata_indent(thislevel);
	printf("next ->\n");
	fsmlist_print_act(l->next, thislevel, cnt);

	automata_indent(level);
	printf("]\n");
}

void
fsmlist_print(struct fsmlist *l)
{
	if (l == NULL) {
		fsmlist_print_act(l, 0, NULL);
		return;
	}
	struct fsmcounter *cnt = fsmcounter_create(l->s);
	fsmlist_print_act(l, 0, cnt);
	fsmcounter_destroy(cnt);
	return;
}

static struct fsmlist*
fsmlist_create(char *name, struct fsm *s)
{
	struct fsmlist *l = (struct fsmlist *) malloc(sizeof(struct fsmlist));
	l->name = name;
	l->s = s;
	l->next = NULL;
	l->tr = circuitbreaker_create(l->s);
	return l;
}

struct fsmlist*
fsmlist_append(struct fsmlist *l, char *name, struct fsm *s)
{
	struct fsmlist *next = fsm_epsclosure(s);
	next->name = name;
	if (l == NULL) {
		return next;
	}
	if (!circuitbreaker_append(l->tr, s)) {
		return l;
	}
	struct fsmlist *tail = fsmlist_tail(l);
	tail->next = next;
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
	circuitbreaker_destroy(l->tr);
	free(l);
}
