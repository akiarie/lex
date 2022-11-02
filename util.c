#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

#include "automata.h"
#include "thompson.h"
#include "util_gen.c"

struct fsm*
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

int
util_numlen(int num)
{
	assert(num >= 0);
	int len = 1;
	for (int k = num; k >= 10; k /= 10) {
		len++;
	}
	return len;
}

#define LEX_NAME_FSM "ln_fsm"
#define LEX_NAME_EDGE "ln_edge"

char*
util_name(char *type, int num)
{
	int len = strlen(type) + util_numlen(num) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s%d", type, num);
	return name;
}

static char*
proper_char(char c)
{
	int len;
	char *s;
	if (c == '\0') {
		len = 2 + 1;
		s = (char *) malloc(sizeof(char) * len);
		snprintf(s, len, "\\0");
	} else {
		len = 1 + 1;
		s = (char *) malloc(sizeof(char) * len);
		snprintf(s, len, "%c", c);
	}
	return s;
}

struct genresult {
	int num;
	char *lval;
};

struct genresult*
genresult_create(int num, char *lval)
{
	struct genresult *r = (struct genresult *) malloc(sizeof(struct genresult));
	r->num = num;
	r->lval = lval;
	return r;
}

void
genresult_destroy(struct genresult *r)
{
	free(r->lval);
	free(r);
}

struct genresult*
util_gen_automaton(struct fsm *s, int num);

static char*
gen_edge_create(char *to, char improperc, bool owner)
{
	char *ownerstr = owner ? "true" : "false";
	char *c = proper_char(improperc);
	int len = strlen("edge_create(, '', )") + strlen(to) + strlen(c) +
		strlen(ownerstr) + 1;
	char *val = (char *) malloc(sizeof(char) * len);
	snprintf(val, len, "edge_create(%s, '%s', %s)", to,
		c, owner ? "true" : "false");
	free(c);
	return val;
}

struct genresult*
util_gen_edge(struct edge *e, int num)
{
	assert(e->dest != NULL);
	struct genresult *r = util_gen_automaton(e->dest, num);
	num = r->num;
	char *command = gen_edge_create(r->lval, e->c, e->owner);
	genresult_destroy(r);
	return genresult_create(r->num, command);
}

struct genresult*
util_gen_automaton(struct fsm *s, int num)
{
	char *name = util_name(LEX_NAME_FSM, num++);
	printf("struct fsm *%s = fsm_create(%s);\n", name,
		s->accepting ? "true" : "false");
	for (int i = 0; i < s->nedges; i++) {
		struct genresult *r = util_gen_edge(s->edges[i], num);
		num = r->num;
		printf("fsm_addedge(%s, %s);\n", name, r->lval);
		genresult_destroy(r);
	}
	return genresult_create(num, name);
}

void
util_gen_driver(char *name)
{
	printf("/* driver code based on %s */\n", name);
}

void
util_gen(struct fsmlist *l, char *varname, FILE *out)
{
	printf("\n/* BEGIN */\n\n");
	struct genresult *r = util_gen_automaton(l->s, 0);
	printf("struct fsm *%s = %s;\n", varname, r->lval);
	genresult_destroy(r);
	printf("\n");
	util_gen_driver(varname);
	printf("\n");
	printf("fsm_destroy(%s);\n", varname);
	printf("\n/* END */\n");
}
