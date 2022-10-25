#ifndef LEX_UTIL
#define LEX_UTIL

struct fsmlist;

struct fsm*
util_fsm_fromstring(char *regex, struct fsmlist *);

#endif
