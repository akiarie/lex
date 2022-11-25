#include<stdlib.h>

#include "parse.h"
#include "automata.h"

struct lexfile *
lexfile_create(char *pre, char *post, struct fsmlist *l, struct token *tokens,
		size_t ntokens)
{
	struct lexfile *f = (struct lexfile *) malloc(sizeof(struct lexfile));
	f->pre = pre;
	f->post = post;
	f->definitions = l;
	f->tokens = tokens;
	f->ntokens = ntokens;
	return f;
}
