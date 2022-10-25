#ifndef LEX_UTIL
#define LEX_UTIL

struct fsm;
struct fsmlist;

struct fsm*
util_fsm_fromstring(char *regex, struct fsmlist *);

void
util_gen(struct fsmlist *, char *varname, FILE *out);

#endif
