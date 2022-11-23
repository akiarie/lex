#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"

/* circuitbreaker: tracker to prevent ε-loops */
struct circuitbreaker {
	struct fsm *s;
	struct circuitbreaker *next;
};

static int
circuitbreaker_len(struct circuitbreaker *tr)
{
	int n = 0;
	for (struct circuitbreaker *next = tr; tr != NULL; tr = tr->next) {
		n++;
	}
	return n;
}


static struct circuitbreaker *
circuitbreaker_create(struct fsm *s)
{
	struct circuitbreaker *tr = (struct circuitbreaker *)
		calloc(1, sizeof(struct circuitbreaker));
	tr->s = s;
	return tr;
}

static struct circuitbreaker *
circuitbreaker_copy(struct circuitbreaker *tr)
{
	struct circuitbreaker *new = circuitbreaker_create(tr->s);
	if (tr->next != NULL) {
		new->next = circuitbreaker_copy(tr->next);
	}
	return new;
}


static void
circuitbreaker_destroy(struct circuitbreaker *tr)
{
	assert(tr != NULL);
	if (tr->next != NULL) {
		circuitbreaker_destroy(tr->next);
	}
	free(tr);
}

static bool
circuitbreaker_append(struct circuitbreaker *tr, struct fsm *s)
{
	for (; tr->s != s; tr = tr->next) {
		if (tr->next == NULL) {
			tr->next = circuitbreaker_create(s);
			return true;
		}
	}
	return false;
}


/* print */

struct fsmcounter {
	struct fsm *p;
	struct fsmcounter *next;
};

static struct fsmcounter *
fsmcounter_create(struct fsm *p)
{
	struct fsmcounter *c = (struct fsmcounter *)
		calloc(1, sizeof(struct fsmcounter));
	c->p = p;
	return c;
}

static void
fsmcounter_destroy(struct fsmcounter *c)
{
	if (c->next != NULL) {
		fsmcounter_destroy(c->next);
	}
	free(c);
}

static int
fsmcounter_count(struct fsmcounter *c, struct fsm *p)
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
indent(int len)
{
	for (int j = 0; j < len; j++) {
		printf("\t");
	}
}

static void
fsm_print_edge_indent(int level)
{
	indent(level);
	printf("|\n");
	indent(level);
}

static void
fsm_print_node(struct fsm *s, int level, int count)
{
	indent(level);
	printf("[%d", count);
	if (s->accepting) {
		printf(", acc");
	}
	printf("](%d) @ 0x%lx\n", (int) s->nedges, (unsigned long) s);
}


static int
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
		printf("* 0x%lx\n", (unsigned long) e->dest);
	}
	return num;
}

static char*
properc(char c)
{
	int len;
	char *n;
	switch (c) {
	case '\0':
		len = strlen("ε") + 1;
		n = (char *) malloc(sizeof(char) * len);
		snprintf(n, len, "ε");
		break;
	case ' ':
		len = 3 + 1;
		n = (char *) malloc(sizeof(char) * len);
		snprintf(n, len, "' '");
		break;
	default:
		len = 2;
		n = (char *) malloc(sizeof(char) * len);
		snprintf(n, len, "%c", c);
		break;
	}
	return n;
}

static int
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

static void
fsmset_print(struct fsmset *l)
{
	printf("fsmset[%d]\n", (int) l->len);
	for (int i = 0; i < l->len; i++) {
		fsm_print(l->arr[i]);
	}
}

/* copymap: a special circuitbreaker for the fsm_copy function that is required
 * because the regular circuitbreaker cannot account for the new addressess */
struct copymap {
	struct fsm *orig, *copy;
	struct copymap *next;
};

static struct copymap *
copymap_create()
{
	return (struct copymap *) calloc(1, sizeof(struct copymap));
}

static void
copymap_destroy(struct copymap *map)
{
	if (map->next != NULL) {
		copymap_destroy(map->next);
	}
	free(map);
}

static struct fsm *
copymap_get(struct copymap *map, struct fsm *orig)
{
	assert(orig != NULL);
	for (; map != NULL; map = map->next) {
		if (map->orig == orig) {
			return map->copy;
		}
	}
	return NULL;
}

static void
copymap_append(struct copymap *map, struct fsm *orig, struct fsm *copy)
{
	assert(map != NULL && orig != NULL && copy != NULL);
	for (; map->next != NULL; map = map->next) {}
	map->next = copymap_create();
	map->next->orig = orig;
	map->next->copy = copy;
}

