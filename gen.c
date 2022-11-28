#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>
#include<string.h>

#include "automata.h"
#include "parse.h"
#include "gen.h"
#include "lex_gen.c"

static void
gen_imports(FILE *out)
{
	for (int i = 0; i < lex_gen_file_len; i++) {
		putchar(lex_gen_file[i]);
	}
}

void
yyin(FILE *out)
{
	fprintf(out,
"char *yyin;\n"
"\n"
"void\n"
"yy_scan_string(char *s)\n"
"{\n"
"	yyin = s;\n"
"}\n");
}

static void
yyfsmlistprep(struct pattern *p, size_t len, FILE *out)
{
	fprintf(out,
"struct pattern *yyfsmlist = NULL;\n"
"\n"
"static void\n"
"yyfsmlistprep() {\n");
	fprintf(out,
"	struct pattern p[] = {\n");
	for (int i = 0; i < len; i++) {
		fprintf(out,
"		{\"%s\",	\"%s\"},\n", p[i].name, p[i].pattern);
	}
	fprintf(out,
"	};\n"
"	for (int i = 0; i < %lu; i++) {\n", len);
	fprintf(out,
"		struct fsm *s = fsm_fromstring(p[i].regex, yyfsmlist);\n"
"		yyfsmlist = fsmlist_append(yyfsmlist, p[i].name, s);\n"
"	};\n"
"}\n");
}

static bool
tokens_hasname(struct token *tokens, size_t len, char *name)
{
	for (int i = 0; i < len; i++) {
		if (strcmp(tokens[i].name, name) == 0) {
			return true;
		}
	}
	return false;
}

/* confirmintegrity: confirms every fsm in l has a matching token in tokens */
static void
confirmintegrity(struct pattern *patterns, size_t npat, struct token *tokens, size_t ntok)
{
	for (int i = 0; i < npat; i++) {
		char *name = patterns[i].name;
		if (!tokens_hasname(tokens, ntok, name)) {
			fprintf(stderr, "'%s' not in tokens\n", name);
			exit(1);
		}
	}
}

static void
yylex(struct pattern *patterns, size_t npat, struct token *tokens, size_t ntok,
		FILE *out)
{
	confirmintegrity(patterns, npat, tokens, ntok);
	yyin(out);
	yyfsmlistprep(patterns, npat, out);
}

void
gen(struct lexer *lx, bool imports, FILE *out)
{
	fprintf(out, "\n/* BEGIN */\n\n");

	if (imports) {
		fprintf(out, "/* BEGIN lex_gen.c */\n");
		gen_imports(out);
		fprintf(out, "/* END lex_gen.c */\n");
	}

	/* preamble */
	fprintf(out, "/* BEGIN preamble */\n");
	fprintf(out, "%s\n", lx->pre);
	fprintf(out, "/* END preamble */\n");

	/* lexer proper */
	fprintf(out, "/* BEGIN lexer */\n");
	yylex(lx->patterns, lx->npat, lx->tokens, lx->ntok, out);
	fprintf(out, "/* END lexer */\n");


	fprintf(out, "/* BEGIN postamble */\n");
	fprintf(out, "%s\n", lx->post);
	fprintf(out, "/* END postamble */\n");

	fprintf(out, "\n/* END */\n");
}
