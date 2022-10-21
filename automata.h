#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

struct edge {
	struct fsm *dest;
	char c;
};

struct edge*
edge_create(struct fsm *, char c);

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

struct fsm*
automata_string_conv(char *);

struct fsm*
automata_concat(struct fsm *, struct fsm *);

struct fsm*
automata_union(struct fsm *, struct fsm *);

struct fsm*
automata_closure(struct fsm *, char closure);

#endif
