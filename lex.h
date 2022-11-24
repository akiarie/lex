#ifndef LEX_LEX
#define LEX_LEX

struct fsm;
struct fsmlist;

struct fsm *
lex_fsm_fromstring(char *regex, struct fsmlist *);

struct token {
	char *tag;
	char *expr;
};

struct pattern {
	char *name;
	bool literal; /* indicates whether the name is surrounded by '{', '}' */
};

struct pattern *
pattern_create(char *name, bool literal);

/* patternlist: pattern-action pairs */
struct patternlist {
	struct pattern *p;
	char *action;
	struct patternlist *next;
};

struct patternlist *
patternlist_append(struct patternlist *pl, struct pattern *p, char *action);

struct lexer {
	char *pre, *post; /* raw sections */
	struct patternlist *pl;
	struct fsmlist *tkl; /* named automata */
};

/* lexer_create: returns a lexer. takes ownership of pre, post and pl and
 * assumes them to be heap-allocated */
struct lexer *
lexer_create(char *pre, char *post, struct patternlist *pl,
		struct fsmlist *tkl);

void
lexer_destroy(struct lexer *);

#endif
