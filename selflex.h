#ifndef LEX_LEX
#define LEX_LEX

enum token_type {
	TOKEN_PUNCT	= 1 << 0,
};

struct terminal {
	enum token_type type;
	char *value;
};

void
lex_string(struct terminal **terminals, int *len, char *input);

#endif
