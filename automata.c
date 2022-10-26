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

struct fsm*
automata_concat_act(struct fsm *s, struct fsm *t, struct sctracker *tr)
{
	assert(s != NULL && t != NULL);
	if (s->accepting) {
		assert(s->nedges == 0);
		s->accepting = false;
		fsm_addedge(s, edge_create(t, '\0', false));
		return s;
	}
	struct sctracker *trnew = sctracker_copy(tr);
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		if (e->dest != t && sctracker_append(trnew, e->dest)) {
			automata_concat_act(e->dest, t, trnew);
		}
	}
	sctracker_destroy(trnew);
	return s;
}


/* r = s·t */
struct fsm*
automata_concat(struct fsm *s, struct fsm *t)
{
	struct sctracker *tr = sctracker_create(s);
	struct fsm *next = automata_concat_act(s, t, tr);
	sctracker_destroy(tr);
	return next;
}

/* r = s | t */
struct fsm*
automata_union(struct fsm *s, struct fsm *t)
{
	struct fsm *start = fsm_create(false);
	fsm_addedge(start, edge_create(s, '\0', true));
	fsm_addedge(start, edge_create(t, '\0', true));
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


struct fsm*
fsm_act_sim(struct fsm *s, struct sctracker *tr, char c);

struct fsm*
edge_traverse(struct edge *e, char c, struct sctracker *tr)
{
	assert(e->dest != NULL);
	if (e->c == '\0') {
		if (sctracker_append(tr, e->dest)) {
			return fsm_act_sim(e->dest, tr, c);
		}
		return NULL;
	}
	return (e->c == c) ? e->dest : NULL;
}


struct fsm*
fsm_act_sim(struct fsm *s, struct sctracker *tr, char c)
{
	assert(s != NULL);
	struct fsm *S = fsm_create(false);
	for (int i = 0; i < s->nedges; i++) {
		struct fsm *next = edge_traverse(s->edges[i], c, tr);
		if (next != NULL) {
			fsm_addedge(S, edge_create(next, '\0', false));
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
		struct fsm *next = edge_traverse(s->edges[i], '\0', tr);
		sctracker_destroy(tr);
		if (fsm_isaccepting(next)) {
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
	fprintf(stderr, "fsm_destroy is NOT IMPLEMENTED\n");
	exit(1);
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
	assert(l->s != NULL && l->name != NULL);
	if (l->next != NULL) {
		fsmlist_destroy(l->next);
	}
	free(l->name);
	fsm_destroy(l->s);
	free(l);
}
