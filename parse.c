#include<stdlib.h>

#include "parse.h"
#include "automata.h"

struct lexer *
lexer_create(char *pre, char *post, struct fsmlist *l, struct token *tokens,
		size_t ntokens)
{
	struct lexer *lx = (struct lexer *) malloc(sizeof(struct lexer));
	lx->pre = pre;
	lx->post = post;
	lx->definitions = l;
	lx->tokens = tokens;
	lx->ntokens = ntokens;
	return lx;
}

void
lexer_destroy(struct lexer *lx)
{
	free(lx->pre);
	free(lx->post);
	fsmlist_destroy(lx->definitions);
	free(lx);
}
