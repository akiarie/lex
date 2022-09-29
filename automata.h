#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA
#include<stdbool.h>

struct edge {
	struct fsm *state;
	char *symbol;
};

struct fsm {
	bool accepting;
	int nedges;
	struct edge **edges;
};

struct fsm*
fsm_create(bool accepting, int nedges);

void
fsm_destroy(struct fsm *);

void
fsm_realloc(struct fsm *, int nedges);

struct fsm*
fsm_trans(struct fsm *state, char *input);

struct fsm*
automata_string_conv(char *);

struct fsm*
automata_concat(struct fsm *, struct fsm *);

struct fsm*
automata_union(struct fsm *, struct fsm *);

struct fsm*
automata_closure(struct fsm *, char closure);

#endif
