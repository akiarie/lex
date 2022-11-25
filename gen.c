#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<assert.h>
#include<string.h>

#include "parse.h"
#include "gen.h"
#include "lex_gen.c"

static int
numlen(int num)
{
	assert(num >= 0);
	int len = 1;
	for (int k = num; k >= 10; k /= 10) {
		len++;
	}
	return len;
}

#define LEX_AUTOMATON "lex_automaton"
#define LEX_FSMLIST "ln_fsmlist"

static void
gen_driver(FILE *out)
{
	fprintf(out, "/* driver code based on %s */\n", LEX_AUTOMATON);
}

static char *
name(char *type, int num)
{
	int len = strlen(type) + numlen(num) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s%d", type, num);
	return name;
}

static void
gen_fsmlist_destroy(FILE *out)
{
	char *lname = name(LEX_FSMLIST, 0);
	fprintf(out,
		"for (struct fsmlist *%s = %s; %s != NULL; %s = %s->next) {\n",
		lname, LEX_AUTOMATON, lname, lname, lname);
	fprintf(out, "\tfsm_destroy(%s->s);\n", lname);
	fprintf(out, "}\n");
	free(lname);
	fprintf(out, "fsmlist_destroy(%s);\n", lname);
}

static void
gen_imports(FILE *out)
{
	for (int i = 0; i < lex_gen_file_len; i++) {
		putchar(lex_gen_file[i]);
	}
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
	fprintf(out, "\n");

	/* generate tokens */
	fprintf(out, "struct token tokens[] = {\n");
	for (struct token *t = lx->tokens; t < lx->tokens + lx->ntokens; t++) {
		/*fprintf(out, "\t{\"%s\",\t\"%s\"},\n", t->tag, t->regex);*/
	}
	fprintf(out, "};\n");

	/* generate fsmlist based on tokens */
	fprintf(out, "struct fsmlist *%s = lexer_create(tokens, %lu);\n",
		LEX_AUTOMATON, lx->ntokens);

	fprintf(out, "\n");

	gen_driver(out);

	fprintf(out, "\n");

	/* destroy objects */
	gen_fsmlist_destroy(out);
	fprintf(out, "fsmlist_destroy(%s);\n", LEX_AUTOMATON);

	fprintf(out, "\n/* END */\n");
}
