#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<strings.h>

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

static char *
substr(char *s, int n)
{
	int len = n + 1;
	char *ss = (char *) malloc(sizeof(char) * len);
	snprintf(ss, len, "%s", s);
	return ss;
}

static char *
parse_id(char *input)
{
	if (!isalpha(*input)) {
		fprintf(stderr, "id must begin with letter: '%s'", input);
		exit(1);
	}
	char *s = input + 1;
	while (isalpha(*s) || isdigit(*s) || *s == '_') {
		s++;
	}
	return substr(input, s - input);
}

static char *
parse_tonewline(char *input)
{
	char *s = input;
	while (*s != '\n') {
		s++;
	}
	return substr(input, s - input);
}


/* skipws: skip whitespace */
static char *
skipws(char *s)
{
	for (; isspace(*s); s++) {}
	return s;
}

static char *
skiplinespace(char *s)
{
	for (; *s == ' ' || *s == '\t'; s++) {}
	return s;
}

struct stringresult {
	char *s;
	char *pos;
};

static struct stringresult
parse_defsraw(char *input)
{
	if (strncmp(input, "%{", 2) != 0) {
		return (struct stringresult){"", input};
	}
	input += 2;
	char *pos = input;
	for (; strncmp(pos, "%}", 2) != 0; pos++) {}
	return (struct stringresult){
		substr(input, pos - input),
		pos + 2,
	};
}

static char *
skipoptions(char *pos)
{
	char *keyword = "%option";
	if (strncmp(pos, keyword, strlen(keyword)) != 0) {
		return pos;
	}
	pos += strlen(keyword);
	pos = skiplinespace(pos);
	char *id = parse_id(pos);
	pos += strlen(id);
	free(id);
	return pos;
}

struct pattern *
pattern_create(char *name, char *pattern)
{
	struct pattern *p = (struct pattern *) malloc(sizeof(struct pattern));
	p->name = name;
	p->pattern = pattern;
	return p;
}

struct patternresult {
	struct pattern *p;
	char *pos;
};

struct patternresult
parse_pattern(char *pos)
{
	char *name = parse_id(pos);
	pos = pos + strlen(name);
	pos = skiplinespace(pos);
	char *pattern = parse_tonewline(pos);
	pos += strlen(pattern);
	return (struct patternresult){pattern_create(name, pattern), pos};
}

struct patternset {
	struct pattern *patterns;
	size_t npat;
	char *pos;
};

static struct patternset
parse_defsproper(char *pos)
{
	size_t npat = 0;
	struct pattern *patterns = NULL;
	for (; strncmp(pos, "%%", 2) != 0 ; npat++) {
		struct patternresult res = parse_pattern(pos);
		pos = res.pos;
		patterns = (struct pattern *)
			realloc(patterns, sizeof(struct pattern) * (npat + 1));
		patterns[npat] = *res.p;
		pos = skipws(pos);
	}
	return (struct patternset){patterns, npat, pos};
}

struct defsresult {
	char *pre;
	struct pattern *patterns;
	size_t npat;
	char *pos;
};

static struct defsresult
parse_defs(char *pos)
{
	pos = skipws(pos);
	if (*pos == '\0') {
		fprintf(stderr, "EOF in defs\n");
		exit(1);
	}
	struct stringresult raw = parse_defsraw(pos);
	pos = raw.pos;
	pos = skipws(pos);
	pos = skipoptions(pos);
	pos = skipws(pos);
	struct patternset set = parse_defsproper(pos);
	return (struct defsresult){
		raw.s, set.patterns, set.npat, set.pos,
	};
}

static struct token *
token_create(char *name, char *action)
{
	struct token *tk = (struct token *) malloc(sizeof(struct token));
	tk->name = name;
	tk->action = action;
	return tk;
}

static struct stringresult
parse_action(char *input)
{
	if (*input != '{') {
		fprintf(stderr, "action must begin with '{' but has '%.*s\n",
			10, input);
		exit(1);
	}
	input++; /* skip '{' */
	char *pos = input;
	for (; *pos != '}'; pos++) {}
	return (struct stringresult){
		substr(input, pos - input),
		pos + 1,
	};

}

static struct stringresult
parse_token_id(char *pos)
{
	char *id = parse_id(++pos); /* skip '{' */
	pos += strlen(id);
	if (*pos != '}') {
		fprintf(stderr, "token id must end in '}' but has '%.*s\n'", 5,
			pos);
		exit(1);
	}
	return (struct stringresult){id, pos + 1}; /* '}' */
}

static struct stringresult
parse_token_literal(char *input)
{
	input++; /* skip '"' */
	char *pos = input;
	for (pos++; *pos != '"'; pos++) {}
	return (struct stringresult){
		substr(input, pos - input),
		pos + 1,
	};
}

static struct stringresult
parse_token_pattern(char *pos)
{
	char *id = parse_id(pos);
	return (struct stringresult){id, pos + strlen(id)};
}

struct tokenresult {
	struct token *tk;
	char *pos;
};

typedef struct stringresult (*nameparser)(char *);

static nameparser
choose_nameparser(char *pos)
{
	switch (*pos) {
	case '{':
		return &parse_token_id;
	case '"':
		return &parse_token_literal;
	default:
		return &parse_token_pattern;
	}
}

static struct tokenresult
parse_token(char *pos)
{
	struct stringresult nameres = choose_nameparser(pos)(pos);
	struct stringresult actionres = parse_action(skiplinespace(nameres.pos));
	return (struct tokenresult){
		token_create(nameres.s, actionres.s),
		actionres.pos,
	};
}

struct rulesresult {
	struct token *tokens;
	size_t ntok;
	char *pos;
};

static struct rulesresult
parse_rules(char *pos)
{
	size_t ntok = 0;
	struct token *tokens = NULL;
	for (; *pos != '\0' && strncmp(pos, "%%", 2) != 0 ; ntok++) {
		struct tokenresult res = parse_token(pos);
		pos = res.pos;
		tokens = (struct token *)
			realloc(tokens, sizeof(struct token) * (ntok + 1));
		tokens[ntok] = *res.tk;
		pos = skipws(pos);
	}
	return (struct rulesresult){tokens, ntok, pos};
}

static char *
parse_toeof(char *input)
{
	char *s = input;
	while (*s != '\0') {
		s++;
	}
	return substr(input, s - input);
}

struct lexer *
parse(char *pos)
{
	struct defsresult def = parse_defs(pos);
	pos = def.pos;
	if (strncmp(pos, "%%", 2) != 0) {
		fprintf(stderr, "invalid transition to rules: '%.*s'\n", 10,
			pos);
		exit(1);
	}
	pos = skipws(pos + 2); /* %% */
	struct rulesresult res = parse_rules(pos);
	pos = res.pos;
	char *post = "";
	if (strncmp(pos, "%%", 2) == 0) {
		pos += 2;
		post = parse_toeof(pos);
	}
	return lexer_create(def.pre, post, def.patterns, def.npat, res.tokens,
		res.ntok);
}
