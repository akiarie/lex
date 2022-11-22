#ifndef LEX_LEX
#define LEX_LEX

struct fsm;
struct fsmlist;

struct fsm *
lex_fsm_fromstring(char *regex, struct fsmlist *);

struct token {
	char *tag;
	char *regex;
};

/* pattern: pattern-action pairs */
struct pattern {
	char *name, *action;
};

struct lexer {
	char *pre, *post; /* raw sections */
	struct pattern *patterns;
	struct fsmlist *l; /* named automata */
};

struct lexer *
lexer_create(struct token *tokens, int len);

void
lexer_destroy(struct lexer *);

#endif
