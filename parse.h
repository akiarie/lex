#ifndef LEX_PARSE
#define LEX_PARSE
#include<stdbool.h>

/* selflex */
struct terminal;

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

/* parse: builds a lexer from the given input, according to the following
 * grammar:
 *
 *	file		→ defs meta rules subs
 *	subs		→ meta raw
 *			| ε
 *
 *	defs		→ defsraw options defsproper
 *	defsraw		→ leftmeta raw rightmeta
 *			| ε
 *	options 	→ '%option' id '\n' options
 *			| ε
 *	defsproper	→ def '\n' defs
 *			| ε
 *	def		→ id regex 	('regex' as defined in thompson.h)
 *
 *	rules		→ pattern { raw } '\n' rules
 *			| ε
 *	pattern		→ {·id·}
 *			| id
 *			| "·string·"	(string potentially containing symbols)
 *
 *	raw		→ .* (until end of environment EOF, '}', '}%', etc.)
 *	id		→ letter (letter | digit)*
 *	meta		→ '\n%%\n'
 *	leftmeta	→ '\n%{\n'
 *	rightmeta	→ '\n}%\n' */
struct lexer *
parse(struct terminal *terminals, int len);

#endif
