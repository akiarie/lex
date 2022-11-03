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

struct fsmlist*
lex_lexer(struct token *tokens, int len);

#endif
