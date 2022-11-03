#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"
#include "thompson.h"
#include "util_gen.c"
#include "util.h"

static struct fsm*
util_fsm_fromtree(struct tnode* tree, struct fsmlist *l)
{
	char *typename;
	struct fsm *start, *final;
	struct tnode* copy;
	switch(tree->type) {
	case NT_EXPR: case NT_UNION:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return util_fsm_fromtree(tree->left, l);
		}
		return automata_union(util_fsm_fromtree(tree->left, l),
			util_fsm_fromtree(tree->right, l));

	case NT_CONCAT: case NT_REST:
		if (tree->right == NULL || tree->right->type == NT_EMPTY) {
			return util_fsm_fromtree(tree->left, l);
		}
		return automata_concat(util_fsm_fromtree(tree->left, l),
			util_fsm_fromtree(tree->right, l), true);

	case NT_CLOSED_BLANK:
		return util_fsm_fromtree(tree->left, l);

	case NT_CLOSURE:
		return automata_closure(util_fsm_fromtree(tree->left, l),
			tree->value[0]);

	case NT_BASIC_EXPR:
		copy = tnode_copy(tree);
		copy->type = NT_EXPR;
		return util_fsm_fromtree(copy, l);

	case NT_BASIC_CLASS:
		return automata_class(tree->value);

	case NT_BASIC_ID:
		return automata_id(tree->value, l);

	case NT_SYMBOL:
		start = fsm_create(false);
		fsm_addedge(start, edge_create(fsm_create(true), tree->value[0],
			true));
		return start;

	default:
		typename = tnode_type_string(tree->type);
		fprintf(stderr, "unknown type %s\n", typename);
		free(typename);
		exit(1);
	}
}


struct fsm*
util_fsm_fromstring(char *input, struct fsmlist *l)
{
	struct tnode *t = thompson_parse(input);
	struct fsm *s = util_fsm_fromtree(t, l);
	tnode_destroy(t);
	return s;
}

static int
util_numlen(int num)
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
util_gen_driver()
{
	printf("/* driver code based on %s */\n", LEX_AUTOMATON);
}

char*
util_name(char *type, int num)
{
	int len = strlen(type) + util_numlen(num) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s%d", type, num);
	return name;
}

static void
util_gen_fsmlist_destroy(FILE *out)
{
	char *lname = util_name(LEX_FSMLIST, 0);
	printf("for (struct fsmlist *%s = %s; %s != NULL; %s = %s->next) {\n",
		lname, LEX_AUTOMATON, lname, lname, lname);
	printf("\tfsm_destroy(%s->s);\n", lname);
	printf("}\n");
	free(lname);
	printf("fsmlist_destroy(%s);\n", lname);
}

void
util_gen(struct token *tokens, int len, FILE *out)
{
	printf("\n/* BEGIN */\n\n");

	/* generate tokens */
	printf("struct { char *name; char *regex; } tokens[] = {\n");
	for (struct token *tk = tokens; tk < tokens + len; tk++) {
		printf("\t{\"%s\",\t\"%s\"},\n", tk->name, tk->regex);
	}
	printf("};\n");

	/* generate fsmlist based on tokens */
	printf("struct fsmlist *%s = NULL;\n", LEX_AUTOMATON);
	printf("for (int i = 0; i < %d; i++) {\n", len);
	printf("\tstruct fsm *s = util_fsm_fromstring(tokens[i].regex, %s);\n",
		LEX_AUTOMATON);
	printf("\t%s = fsmlist_append(%s, tokens[i].name, s);\n",
		LEX_AUTOMATON, LEX_AUTOMATON);
	printf("};\n");

	printf("\n");

	util_gen_driver();

	printf("\n");

	/* destroy objects */
	util_gen_fsmlist_destroy(out);
	printf("fsmlist_destroy(%s);\n", LEX_AUTOMATON);

	printf("\n/* END */\n");
}
