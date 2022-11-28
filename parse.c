#include<stdlib.h>

#include "parse.h"
#include "automata.h"

struct lexer *
lexer_create(char *pre, char *post, struct pattern *patterns, size_t npat,
		struct token *tokens, size_t ntok)
{
	struct lexer *lx = (struct lexer *) malloc(sizeof(struct lexer));
	lx->pre = pre;
	lx->post = post;
	lx->patterns = patterns;
	lx->npat = npat;
	lx->tokens = tokens;
	lx->ntok = ntok;
	return lx;
}

void
lexer_destroy(struct lexer *lx)
{
	free(lx->pre);
	free(lx->post);
	free(lx);
}
