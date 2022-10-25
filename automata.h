#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

struct edge {
	struct fsm *dest;
	bool owner;
	char c;
};

struct fsm {
	bool accepting;
	int nedges;
	struct edge **edges;
};

struct fsm*
fsm_create(bool accepting);

void
fsm_destroy(struct fsm *);

void
fsm_addedge(struct fsm *, struct edge *);

/* fsm_sim: simulate the fsm by returning the next state on the given input
 * (which might be NULL) */
struct fsm*
fsm_sim(struct fsm *state, char c);

bool
fsm_isaccepting(struct fsm *state);

int
fsm_print(struct fsm *);

struct fsmlist {
	char *name;
	struct fsm *s;
	struct fsmlist *next;
};

struct fsmlist*
fsmlist_create(char *name, char *regex, struct fsmlist *base);

void
fsmlist_destroy(struct fsmlist *);

void
fsmlist_append(struct fsmlist *, struct fsmlist *);

#endif
