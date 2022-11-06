#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

/* edge */

struct fsm;

struct edge {
	char c;
	struct fsm *dest;
	bool owner;
};

struct edge *
edge_create(struct fsm *dest, char c, bool owner);

/* fsm */

struct fsm {
	bool accepting;
	size_t nedges;
	struct edge **edges;
};

struct fsm *
fsm_create(bool accepting);

void
fsm_destroy(struct fsm *);

void
fsm_addedge(struct fsm *, struct edge *);

struct fsm *
fsm_sim(struct fsm *, char);

void
fsm_print(struct fsm *);

struct fsmset {
	struct fsm **arr;
	size_t len;
};

/* fsmlist */

struct fsmlist {
	char *name;
	struct fsm *s;
	struct fsmlist *next;
};

void
fsmlist_destroy(struct fsmlist *);

struct fsmlist *
fsmlist_append(struct fsmlist *, char *, struct fsm *);

/* automata */

void
automata_concat(struct fsm *, struct fsm *, bool);

struct fsm *
automata_union(struct fsm *, struct fsm *);

struct fsm *
automata_closure(struct fsm *, char);

struct fsm *
automata_class(char *);

struct fsm *
automata_id(char *id, struct fsmlist *l);

#endif
