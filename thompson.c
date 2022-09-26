#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<ctype.h>
#include<assert.h>

#include "thompson.h"

bool
thompson_atend(char *input)
{
	switch (input[0]) {
	case '\0': case ')': case ']': case '}':
		return true;
	}
	return false;
}

struct tnode*
thompson_symbol(char *input)
{
	char c = input[0];
	if (isalpha(c) || isdigit(c)) {
		struct tnode *this = tnode_create(NT_SYMBOL);
		this->c = c;
		this->len = 1;
		this->output = (char*) realloc(this->output, sizeof(char) * 2);
		snprintf(this->output, 2, "%c", c);
		return this;
	}
	return NULL;
}

void
thompson_validate_range(char start, char end)
{
	if ( (isdigit(start) && isdigit(end))
			|| (isupper(start) && isupper(end))
			|| (islower(start) && islower(end)) ) {
		if (start <= end) {
			return;
		}
		fprintf(stderr, "'%c' higher than '%c'\n", start, end);
		exit(1);
	}
	fprintf(stderr, "'%c' and '%c' cannot be limits\n", start, end);
	exit(1);
}

struct tnode*
thompson_atom(char *input)
{
	char *pos = input;
	struct tnode *l = thompson_symbol(pos);
	if (l == NULL) {
		fprintf(stderr, "atom must begin with symbol\n");
		exit(1);
	}
	pos += l->len;
	if (pos[0] != '-') {
		return l;
	}
	pos++;
	struct tnode *r = thompson_symbol(pos);
	if (r == NULL) {
		fprintf(stderr, "range must end with symbol\n");
		exit(1);
	}
	pos += r->len;

	thompson_validate_range(l->c, r->c);

	struct tnode *this = tnode_create(NT_RANGE);
	this->len = pos - input;
	this->left = l;
	this->right = r;

	int outlen = strlen(l->output) + strlen(r->output) + 1 + 1; // '-'
	this->output = (char*) realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s-%s", l->output, r->output);
	return this;
}

struct tnode*
thompson_inclass(char *input)
{
	char *pos = input;
	if (pos[0] == ']') { // ε
		return tnode_create(NT_INCLASS);
	}

	struct tnode *l = thompson_atom(pos);
	pos += l->len;
	struct tnode *r = thompson_inclass(pos);
	pos += r->len;

	struct tnode *this = tnode_create(NT_INCLASS);
	this->len = pos - input;
	this->left = l;
	this->right = r;
	int outlen = strlen(l->output) + strlen(r->output) + 1;
	this->output = (char*) realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s%s", l->output, r->output);
	return this;
}

struct tnode*
thompson_class(char *input)
{
	struct tnode *this = tnode_create(NT_CLASS);
	char *invsym = "";
	char *pos = input;
	if (pos[0] == '^') {
		this->c = '^';
		invsym = "^";
		pos++;
	}
	struct tnode *icl = thompson_inclass(pos);
	pos += icl->len;
	this->left = icl;
	this->len = pos - input;

	int outlen = strlen(invsym) + strlen(icl->output) + 1;
	this->output = realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s%s", invsym, icl->output);
	return this;
}


struct tnode*
thompson_basic(char *input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_BASIC_EMPTY);
	}
	char *pos = input;
	int outlen;
	char *output;
	enum tnode_type type;
	struct tnode *child;
	switch (input[0]) {
	case '(':
		type = NT_BASIC_BRACKET;
		pos++;
		child = thompson_parse(pos);
		pos += child->len;
		if (pos[0] != ')') {
			fprintf(stderr, "expr bracket not closed\n");
			exit(1);
		}
		pos++;
		outlen = strlen(child->output) + 2 + 1; // '(' ')'
		output = (char*) malloc(sizeof(char) * outlen);
		snprintf(output, outlen, "(%s)", child->output);
		break;
	case '[':
		type = NT_BASIC_CLASS;
		pos++;
		child = thompson_class(pos);
		pos += child->len;
		if (pos[0] != ']') {
			fprintf(stderr, "class bracket not closed\n");
			exit(1);
		}
		pos++;
		outlen = strlen(child->output) + 2 + 1; // '[' ']'
		output = (char*) malloc(sizeof(char) * outlen);
		snprintf(output, outlen, "[%s]", child->output);
		break;
	default:
		type = NT_BASIC_SYMBOL;
		child = thompson_symbol(pos);
		if (child == NULL) {
			return NULL;
		}
		pos += child->len;
		outlen = strlen(child->output) + 1;
		output = (char*) calloc(1, sizeof(char) * outlen);
		snprintf(output, outlen, "%s", child->output);
		break;
	}
	struct tnode *this = tnode_create(type);
	this->left = child;
	this->len = pos - input;
	free(this->output);
	this->output = output;
	return this;
}


struct tnode*
thompson_closed(char *input)
{
	char *pos = input;
	struct tnode *basic = thompson_basic(pos);
	if (basic == NULL) {
		return NULL;
	}
	pos += basic->len;
	char c = pos[0];
	char *output = basic->output;
	if (!thompson_atend(pos)) {
		if (c == '*' || c == '+') {
			pos += 1;
			char d = pos[0];
			if (!thompson_atend(pos) && (d == '*' || d == '+')) {
				fprintf(stderr, "double closures not allowed\n");
				exit(1);
			}
			int len = strlen(basic->output) + 1 + 1;
			output = (char*) malloc(sizeof(char) * len);
			snprintf(output, len, "%s%c", basic->output, c);
		}
	}
	struct tnode *this = tnode_create(NT_CLOSED);
	this->c = c;
	this->left = basic;
	this->len = pos - input;
	int outlen = strlen(output) + 1; // '.'
	this->output = realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s", output);
	return this;
}


struct tnode*
thompson_rest(char *input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_REST_EMPTY);
	}

	char *pos = input;
	struct tnode *l = thompson_closed(pos);
	if (l == NULL) {
		return tnode_create(NT_REST_EMPTY);
	}
	pos += l->len;
	struct tnode *r = thompson_rest(pos);
	if (r == NULL) {
		return tnode_create(NT_REST_EMPTY);
	}
	pos += r->len;

	struct tnode *this = tnode_create(NT_REST);
	this->left = l;
	this->right = r;
	this->len = pos - input;

	int outlen = strlen(l->output) + strlen(r->output) + 1 + 1; // '.'
	this->output = realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, ".%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_concat(char *input)
{
	char *pos = input;
	struct tnode *l = thompson_closed(pos);
	pos += l->len;
	struct tnode *r = thompson_rest(pos);
	pos += r->len;

	struct tnode *this = tnode_create(NT_CONCAT);
	this->left = l;
	this->right = r;
	this->len = pos - input;

	int outlen = strlen(l->output) + strlen(r->output) + 1;
	this->output = (char*) realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_union(char *input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_UNION_EMPTY);
	}

	if (input[0] != '|') {
		fprintf(stderr, "nonempty unions must start with '|'\n");
		exit(1);
	}

	char *pos = input + 1;
	struct tnode *l = thompson_concat(pos);
	pos += l->len;
	struct tnode *r = thompson_union(pos);
	pos += r->len;

	struct tnode *this = tnode_create(NT_UNION);
	this->left = l;
	this->right = r;
	this->len = pos - input;

	int outlen = strlen(l->output) + strlen(r->output) + 1 + 1; // '|'
	this->output = (char*) realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "|%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_parse(char *input)
{
	char *pos = input;
	struct tnode *l = thompson_concat(pos);
	pos += l->len;
	struct tnode *r = thompson_union(pos);
	pos += r->len;

	struct tnode *this = tnode_create(NT_EXPR);
	this->left = l;
	this->right = r;
	this->len = pos - input;

	int outlen = strlen(l->output) + strlen(r->output) + 1;
	this->output = (char*) realloc(this->output, sizeof(char) * outlen);
	snprintf(this->output, outlen, "%s%s", l->output, r->output);
	return this;
}


struct tnode*
tnode_create(enum tnode_type type)
{
	struct tnode *this = (struct tnode*) malloc(sizeof(struct tnode));
	this->left = this->right = NULL;
	this->type = type;
	this->len = 0;
	this->output = (char*) calloc(1, sizeof(char));
	return this;
}

void
tnode_printf(struct tnode *this)
{
	printf("{len: %d\tc: %c\toutput: %s}\n", this->len, this->c,
		this->output);
}

void
tnode_destroy(struct tnode *this)
{
	if (this->left != NULL) {
		tnode_destroy(this->left);
	}
	if (this->right != NULL) {
		tnode_destroy(this->right);
	}
	free(this->output);
}
