#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

struct edge {
	struct fsm *dest;
	bool owner;
	char c;
};

struct edge*
edge_create(struct fsm*, char, bool);

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

void
fsm_print(struct fsm *);

struct fsmlist {
	char *name;
	struct fsm *s;
	struct fsmlist *next;
};

/* creates a list if NULL pointer is supplied */
struct fsmlist*
fsmlist_append(struct fsmlist *, char *name, struct fsm *s);

void
fsmlist_destroy(struct fsmlist *);

struct fsmlist*
fsmlist_copy(struct fsmlist *);

struct fsm*
automata_concat(struct fsm *, struct fsm *);

struct fsm*
automata_union(struct fsm *, struct fsm *);

struct fsm*
automata_closure(struct fsm *, char closure);

struct fsm*
automata_class(char *value);

struct fsm*
automata_id(char *id, struct fsmlist *l);

#endif
