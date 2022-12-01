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
		fputc(lex_gen_file[i], out);
	}
}

void
yyin(FILE *out)
{
	fprintf(out,
"char *yyin = NULL;\n"
"\n"
"void\n"
"yy_scan_string(char *s)\n"
"{\n"
"	int len = strlen(s) + 1;\n"
"	yyin = (char *) malloc(sizeof(char) * len);\n"
"	snprintf(yyin, len, \"%%s\", s);\n"
"}\n");
}

static void
fprintliteral(FILE *out, char *s)
{
	int len = strlen(s);
	fprintf(out, "(char []){");
	for (int i = 0; i < len; i++) {
		fprintf(out, "%d,", s[i]);
	}
	fprintf(out, "'\\0'} /* %s */", s);
}

static void
yyfsmlistprep(FILE *out, struct pattern *p, size_t npat, struct token *t,
		size_t ntok)
{
	fprintf(out,
"static struct fsm *\n"
"gettokenfsm(struct token *t, struct fsmlist *l)\n"
"{\n"
"	if (t->literal) {\n"
"		/* TODO: create token */\n"
"		fprintf(stderr, \"literal tokens NOT IMPLEMENTED\\n\");\n"
"		exit(1);\n"
"	}\n"
"	struct fsm *s = fsmlist_findfsm(l, t->name);\n"
"	if (NULL == s) {\n"
"		fprintf(stderr, \"cannot find pattern for '%%s'\\n\", t->name);\n"
"		exit(1);\n"
"	}\n"
"	return fsm_copy(s);\n"
"}\n"
"\n"
"struct fsmlist *yyfsmlist = NULL;\n"
"unsigned long yyleng = 0;\n"
"char *yytext = NULL;\n"
"\n"
"static void\n"
"yyfsmlistprep()\n"
"{\n");
	fprintf(out,
"	struct pattern p[] = {\n");
	for (int i = 0; i < npat; i++) {
		fprintf(out,
"		{\"%s\",\t", p[i].name);
		fprintliteral(out, p[i].pattern);
		fprintf(out, "},\n");
	}
	fprintf(out,
"	};\n"
"	struct fsmlist *l = NULL;\n"
"	for (int i = 0; i < %lu; i++) {\n", npat);
	fprintf(out,
"		struct fsm *s = fsm_fromstring(p[i].pattern, l);\n"
"		l = fsmlist_append(l, p[i].name, s);\n"
"	};\n"
"	struct token t[] = {\n");
	for (int i = 0; i < ntok; i++) {
		fprintf(out,
"		{%s,\t\"%s\",\t", (t[i].literal ? "true" : "false"), t[i].name);
		fprintliteral(out, t[i].action);
		fprintf(out, "},\n");
	}
	fprintf(out,
"	};\n"
"	for (int i = 0; i < %lu; i++) {\n", ntok);
	fprintf(out,
"		yyfsmlist = fsmlist_append(yyfsmlist, t[i].name, gettokenfsm(&t[i], l));\n"
"	};\n"
"	fsmlist_destroy(l);\n"
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

static void
yylex(FILE *out, struct pattern *p, size_t npat, struct token *t, size_t ntok)
{
	yyin(out);
	fprintf(out,
"\n");
	yyfsmlistprep(out, p, npat, t, ntok);
	fprintf(out,
"\n"
"int\n"
"yylex()\n"
"{\n");
	fprintf(out,
"	if (NULL == yyfsmlist) {\n"
"		yyfsmlistprep();\n"
"	}\n"
"	struct findresult *r = fsmlist_findnext(yyfsmlist, yyin);\n"
"	yyleng = r->len; yytext = yyin;\n"
"	yyin += yyleng;\n"
"	if (yyleng == 0) {\n"
"		assert(*yyin == '\\0');\n"
"		return 0;\n"
"	}\n"
"	if (r->fsm == NULL) {\n"
"		fprintf(stderr, \"unmatched '%%.*s'\\n\", (int) yyleng, yytext);\n"
"		exit(1);\n"
"	}\n"
"	/* ⊢ r->fsm != NULL && yyleng > 0 */\n");
	for (int i = 0; i < ntok; i++) {
		char *els = i == 0 ? "" : "} else ";
		struct token tk = t[i];
		if (i == 0) {
			fprintf(out,
"	if (strcmp(r->fsm, \"%s\") == 0) {\n"
"		%s\n", tk.name, tk.action);
		} else {
			fprintf(out,
"	} else if (strcmp(r->fsm, \"%s\") == 0) {\n"
"		%s\n", tk.name, tk.action);
		}
	}
	fprintf(out,
"	}\n"
"	/* recurse if no return-action above */\n"
"	return yylex();\n"
"}\n");
}

void
gen(FILE *out, struct lexer *lx, bool imports)
{
	fprintf(out,
"/* BEGIN */\n"
"\n");
	if (imports) {
		fprintf(out,
"/* BEGIN lex_gen.c */\n");
		gen_imports(out);
		fprintf(out,
"/* END lex_gen.c */\n");
	}
	fprintf(out,
"/* BEGIN preamble */\n"
"%s\n"
"/* END preamble */\n", lx->pre);
	fprintf(out,
"/* BEGIN lexer */\n");
	yylex(out, lx->patterns, lx->npat, lx->tokens, lx->ntok);
	fprintf(out,
"/* END lexer */\n");
	fprintf(out,
"/* BEGIN postamble */\n"
"%s\n"
"/* END postamble */\n", lx->post);
	fprintf(out,
"\n"
"/* END */\n");
}
