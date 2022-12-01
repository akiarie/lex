#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdlib.h>
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

struct fsmlist;

struct fsm *
fsm_fromstring(char *regex, struct fsmlist *);

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

struct fsm *
fsmlist_findfsm(struct fsmlist *, char *name);

struct fsmlist *
fsmlist_append(struct fsmlist *, char *, struct fsm *);

struct findresult {
	char *fsm;
	unsigned long len;
};

void
findresult_destroy(struct findresult *r);

/* fsmlist_findnext: simulates the fsms with the input until it can determine
 * the highest-ranking, next match. if no match is found, returns a findresult
 * with fsm == NULL and len set to the number of chars scanned. l is invariant */
struct findresult *
fsmlist_findnext(struct fsmlist *l, char *input);


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
