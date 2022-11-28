#ifndef LEX_PARSE
#define LEX_PARSE

struct fsmlist;

struct token { char *name, *action; };

struct lexer {
	char *pre, *post;
	struct fsmlist *definitions;
	struct token *tokens;
	size_t ntokens;
};

/* lexer_create: returns a lexer. takes ownership of pre, post and l. */
struct lexer *
lexer_create(char *pre, char *post, struct fsmlist *l, struct token *, size_t);

void
lexer_destroy(struct lexer *);

#endif
