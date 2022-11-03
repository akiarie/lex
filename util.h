#ifndef LEX_UTIL
#define LEX_UTIL

struct fsm;
struct fsmlist;

struct fsm*
util_fsm_fromstring(char *regex, struct fsmlist *);

struct token {
	char *name;
	char *regex;
};

void
util_gen(struct token *tokens, int len, FILE *out);

#endif
