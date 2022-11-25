#ifndef LEX_PARSE
#define LEX_PARSE

struct fsmlist;

struct token { char *name, *action; };

struct lexfile {
	char *pre, *post;
	struct fsmlist *definitions;
	struct token *tokens;
	size_t ntokens;
};

struct lexfile *
lexfile_create(char *, char *, struct fsmlist *, struct token *, size_t);

#endif
