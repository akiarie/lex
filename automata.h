#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

struct edge {
	char *symbol;
	struct fsm **states[];
};

struct fsm {
	bool accepting;
	struct edge **edges[];
};

struct fsm*
fsm_create(bool accepting, struct edge *edges[]);

void
fsm_destroy(struct fsm *);

struct fsm*
fsm_trans(struct fsm *state, char *input);

struct fsm*
automata_fromstring(char *);

struct fsm*
automata_concat(struct fsm *, struct fsm *);

struct fsm*
automata_or(struct fsm *, struct fsm *);

struct fsm*
automata_closure(struct fsm *, char closure);

#endif
