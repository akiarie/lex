#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"


/* sctracker: short circuit tracker */
struct sctracker {
	void *s;
	struct sctracker *next;
};


struct sctracker*
sctracker_create(void *s)
{
	struct sctracker *tr = (struct sctracker *)
		malloc(sizeof(struct sctracker));
	tr->s = s;
	tr->next = NULL;
	return tr;
}


struct sctracker*
sctracker_copy(struct sctracker *tr)
{
	struct sctracker *new = sctracker_create(tr->s);
	if (tr->next != NULL) {
		new->next = sctracker_copy(tr->next);
	}
	return new;
}


void
sctracker_destroy(struct sctracker *tr)
{
	assert(tr != NULL);
	if (tr->next != NULL) {
		sctracker_destroy(tr->next);
	}
	free(tr);
}


bool
sctracker_append(struct sctracker *tr, void *s)
{
	for (; tr->s != s; tr = tr->next) {
		if (tr->next == NULL) {
			tr->next = sctracker_create(s);
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
	int owners = owner ? 0 : 1;
	for (struct fsmlist *l = fsm_finals(s); l != NULL; l = l->next) {
		assert(l->s->accepting);
		l->s->accepting = false;
		fsm_addedge(l->s, edge_create(t, '\0', owners++ > 0));
	}
	return s;
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(t, '\0', true));
	struct fsm *final = fsm_create(true);
	automata_concat(s, final, true);
	automata_concat(t, final, false);
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
		closure = automata_concat(closure, closure, false);
		return closure;
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
			return l->s;
		}
	}
	fprintf(stderr, "unknown pattern '%s' has not been declared\n", id);
	exit(1);
}

struct fsmlist*
fsm_finals_act(struct fsm *s, struct sctracker *tr)
{
	struct fsmlist *l = NULL;
	if (s->accepting) {
		l = fsmlist_append(l, NULL, s);
	}
	struct sctracker *trnew = sctracker_copy(tr);
	for (int i = 0; i < s->nedges; i++) {
		struct edge  *e = s->edges[i];
		assert(e != NULL);
		if (e->c == '\0' && !sctracker_append(trnew, e->dest)) {
			continue;
		}
		for (struct fsmlist *m = fsm_finals_act(e->dest, trnew); m != NULL;
				m = m->next) {
			l = fsmlist_append(l, m->name, m->s);
		}
	}
	sctracker_destroy(trnew);
	return l;
}

struct fsmlist*
fsm_finals(struct fsm *s)
{
	struct sctracker *tr = sctracker_create(s);
	struct fsmlist *l = fsm_finals_act(s, tr);
	sctracker_destroy(tr);
	assert(l != NULL);
	return l;
}

struct fsmlist*
fsm_epsclosure_act(struct fsm *s, struct sctracker *tr)
{
	struct fsmlist *l = fsmlist_append(NULL, NULL, s);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		assert(e != NULL);
		if (e->c != '\0' || !sctracker_append(tr, e->dest)) {
			continue;
		}
		for (struct fsmlist *m = fsm_epsclosure_act(e->dest, tr);
				m != NULL; m = m->next) {
			fsmlist_append(l, m->name, m->s);
		}
	}
	return l;
}

struct fsmlist*
fsm_epsclosure(struct fsm *s)
{
	struct sctracker *tr = sctracker_create(s);
	struct fsmlist *l = fsm_epsclosure_act(s, tr);
	sctracker_destroy(tr);
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

void
fsm_destroy(struct fsm *s)
{
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

int
fsm_print_act(struct fsm *s, int level, int thisnum, struct sctracker *tr)
{
	assert(s != NULL);
	int num = thisnum;
	automata_indent(level);
	if (s->accepting) {
		printf("[%d, acc](%d)\n", num, s->nedges);
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
		printf("-- %c -->\n", e->c);
		struct sctracker *trnew = sctracker_copy(tr);
		if (sctracker_append(trnew, s->edges[i])) {
			num += fsm_print_act(e->dest, level + 1, num + 1, trnew);
		}
		sctracker_destroy(trnew);
	}
	return s->nedges;
}

void
fsm_print(struct fsm *s)
{
	struct sctracker *tr = sctracker_create(s);
	fsm_print_act(s, 0, 0, tr);
	sctracker_destroy(tr);
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
		fsmlist_append(m, n->name, n->s);
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
		fsmlist_append(m, n->name, n->s);
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
fsmlist_create(char *name, struct fsm *s)
{
	struct fsmlist *l = (struct fsmlist *) malloc(sizeof(struct fsmlist));
	l->name = name;
	l->s = s;
	l->next = NULL;
	return l;
}

struct fsmlist*
fsmlist_tail(struct fsmlist *l)
{
	for (; l->next != NULL; l = l->next) {}
	return l;
}

struct fsmlist*
fsmlist_append(struct fsmlist *l, char *name, struct fsm *s)
{
	struct fsmlist *next = fsmlist_create(name, s);
	if (l == NULL) {
		return next;
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
	assert(l->s != NULL && l->name != NULL);
	if (l->next != NULL) {
		fsmlist_destroy(l->next);
	}
	if (l->name != NULL) {
		free(l->name);
	}
	fsm_destroy(l->s);
	free(l);
}
