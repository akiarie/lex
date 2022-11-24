#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<assert.h>

#include "automata.h"
#include "lex.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

static char *
dynamic_name(char *static_name)
{
	assert(static_name != NULL);
	int len = strlen(static_name) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s", static_name);
	return name;
}

struct fsmlist *
gettokenlist(struct token *tokens, int len)
{
	struct fsmlist *l = NULL;
	for (int i = 0; i < len; i++) {
		struct token tk = tokens[i];
		struct fsm *s = lex_fsm_fromstring(tk.expr, l);
		l = fsmlist_append(l, tk.tag, s);
	}
	return l;
}

struct patternlist *
getpatternlist(struct pattern *patterns, int len)
{
	struct patternlist *pl = NULL;
	for (int i = 0; i < len; i++) {
	}
	return pl;
}

void
run()
{
	char *pre = dynamic_name("/* preamble */");
	char *post = dynamic_name("/* postamble */");
	struct token tokens[] = {
		{"ws",		"[ ]"},
		{"vowel",	"[aeiou]"},
		{"vowelb",	"{vowel}b"},
	};
	struct pattern patterns[] = {
		{"{ws}",	"{ /* action for {ws} */ }"},
		{"ab",		"{ /* action for ab */ }"},
		{"{vowelb}",	"{ /* action for {vowelb} */ }"},
	};
	struct lexer *lx = lexer_create(
		pre,
		post,
		getpatternlist(patterns, LEN(patterns)),
		gettokenlist(tokens, LEN(tokens))
	);
	lexer_destroy(lx);
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		run,
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}
}
