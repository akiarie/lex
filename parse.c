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
		printf("pattern: %s, %s\n", res.p->name, res.p->pattern);
	}
	fprintf(stderr, "parse_defsproper NOT IMPLEMENTED\n");
	exit(1);
}

static struct {
	char *pre;
	struct pattern *patterns;
	size_t npat;
} defs;

static char *
parse_defs(char *pos)
{
	pos = skipws(pos);
	if (*pos == '\0') {
		fprintf(stderr, "EOF in defs\n");
		exit(1);
	}
	struct stringresult raw = parse_defsraw(pos);
	defs.pre = raw.s;
	pos = raw.pos;
	pos = skipws(pos);
	pos = skipoptions(pos);
	pos = skipws(pos);
	struct patternset set = parse_defsproper(pos);
	defs.patterns = set.patterns;
	defs.npat = set.npat;
	return set.pos;
}

struct lexer *
parse(char *input)
{
	char *pos = parse_defs(input);
	for (int i = 0; i < defs.npat; i++) {
		struct pattern p = defs.patterns[i];
		printf("%s\t%s\n", p.name, p.pattern);
	}
	return NULL;
}
