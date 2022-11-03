#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "lex.h"

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

void
gen(struct token *tokens, int len, FILE *out)
{
	printf("\n/* BEGIN */\n\n");

	/* generate tokens */
	printf("struct { char *name; char *regex; } tokens[] = {\n");
	for (struct token *tk = tokens; tk < tokens + len; tk++) {
		printf("\t{\"%s\",\t\"%s\"},\n", tk->tag, tk->regex);
	}
	printf("};\n");

	/* generate fsmlist based on tokens */
	printf("struct fsmlist *%s = NULL;\n", LEX_AUTOMATON);
	printf("for (int i = 0; i < %d; i++) {\n", len);
	printf("\tstruct fsm *s = fsm_fromstring(tokens[i].regex, %s);\n",
		LEX_AUTOMATON);
	printf("\t%s = fsmlist_append(%s, tokens[i].name, s);\n",
		LEX_AUTOMATON, LEX_AUTOMATON);
	printf("};\n");

	printf("\n");

	gen_driver();

	printf("\n");

	/* destroy objects */
	gen_fsmlist_destroy(out);
	printf("fsmlist_destroy(%s);\n", LEX_AUTOMATON);

	printf("\n/* END */\n");
}
