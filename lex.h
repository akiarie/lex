#ifndef LEX_LEX
#define LEX_LEX

struct fsm;
struct fsmlist;

struct fsm*
lex_fsm_fromstring(char *regex, struct fsmlist *);

struct token {
	char *tag;
	char *regex;
};

struct lexer {
	struct fsmlist *l;
	char *input;
	int pos;
};

struct lexer*
lexer_create(struct fsmlist *, char *);

void
lexer_destroy(struct lexer *);

struct lexer*
lex(struct token *tokens, int len, char *);

#endif
