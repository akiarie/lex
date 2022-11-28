#ifndef LEX_PARSE
#define LEX_PARSE
#include<stdbool.h>

struct fsmlist;

struct pattern { char *name, *pattern; };
struct token { bool literal; char *name, *action; };

struct lexer {
	char *pre, *post;
	struct pattern *patterns;
	size_t npat;
	struct token *tokens;
	size_t ntok;
};

/* lexer_create: returns a lexer. takes ownership of pre, post and l. */
struct lexer *
lexer_create(char *, char *, struct pattern *, size_t, struct token *, size_t);

void
lexer_destroy(struct lexer *);

#endif
