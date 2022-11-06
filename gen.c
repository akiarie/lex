#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "lex.h"
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
gen_driver()
{
	printf("/* driver code based on %s */\n", LEX_AUTOMATON);
}

static char*
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
	printf("for (struct fsmlist *%s = %s; %s != NULL; %s = %s->next) {\n",
		lname, LEX_AUTOMATON, lname, lname, lname);
	printf("\tfsm_destroy(%s->s);\n", lname);
	printf("}\n");
	free(lname);
	printf("fsmlist_destroy(%s);\n", lname);
}

static void
libraries(FILE *out)
{
	for (int i = 0; i < lex_gen_file_len; i++) {
		putchar(lex_gen_file[i]);
	}
}

void
gen(struct token *tokens, int len, FILE *out)
{
	printf("\n/* BEGIN */\n\n");

	printf("/* BEGIN lex_gen.c */\n");
	libraries(out);
	printf("/* END lex_gen.c */\n");
	printf("\n");

	/* generate tokens */
	printf("struct { char *name; char *regex; } tokens[] = {\n");
	for (struct token *t = tokens; t < tokens + len; t++) {
		printf("\t{\"%s\",\t\"%s\"},\n", t->tag, t->regex);
	}
	printf("};\n");

	/* generate fsmlist based on tokens */
	printf("struct fsmlist *%s = lex_lexer(tokens, %d);\n", LEX_AUTOMATON,
		len);

	printf("\n");

	gen_driver();

	printf("\n");

	/* destroy objects */
	gen_fsmlist_destroy(out);
	printf("fsmlist_destroy(%s);\n", LEX_AUTOMATON);

	printf("\n/* END */\n");
}
