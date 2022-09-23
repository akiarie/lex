#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "thompson.h"

bool
thompson_atend(char* input)
{
	switch (input[0]) {
	case '\0': case ')': case ']':
		return true;
	}
	return false;
}


struct tnode*
thompson_symbol(char* input)
{
	char c = input[0];
	if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')
			|| ('0' <= c && c <= '9')) {
		struct tnode *this = tnode_create(NT_SYMBOL);
		this->c = c;
		this->len = 1;
		this->output = (char*) realloc(this->output, sizeof(char) + 1);
		snprintf(this->output, 1, "%c", c);
		return this;
	}
	return NULL;
}


struct tnode*
thompson_class(char* input)
{
	fprintf(stderr, "class not implemented");
	exit(1);
}


struct tnode*
thompson_basic(char* input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_BASIC_EMPTY);
	}
	struct tnode *this;
	char *pos = input;
	switch (input[0]) {
	case '(':
		this = tnode_create(NT_BASIC_BRACKET);
		pos++; printf("(");
		this->left = thompson_parse(pos); pos += this->left->len;
		if (pos[0] != ')') {
			fprintf(stderr, "expr bracket not closed");
			exit(1);
		}
		pos++; printf(")");
		break;
	case '[':
		this = tnode_create(NT_BASIC_CLASS);
		pos++; printf("[");
		this->left = thompson_class(pos); pos += this->left->len;
		if (pos[0] != ']') {
			fprintf(stderr, "class bracket not closed");
			exit(1);
		}
		pos++; printf("]");
		break;
	default:
		this = tnode_create(NT_BASIC_SYMBOL);
		this->left = thompson_symbol(pos); pos += this->left->len;
		break;
	}
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_closed(char* input)
{
	struct tnode *this = tnode_create(NT_CLOSED);
	char *pos = input;
	this->left = thompson_basic(pos); pos += this->left->len;
	if (!thompson_atend(pos)) {
		if (this->c == '*' || this->c == '+') {
			this->c = pos[0];
			pos += 1;
			char d = pos[0];
			if (!thompson_atend(pos) && (d == '*' || d == '+')) {
				fprintf(stderr, "double closures not allowed");
				exit(1);
			}
			printf("%c", this->c);
		}
	}
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_rest(char* input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_REST_EMPTY);
	}

	char *pos = input;
	struct tnode *l = thompson_closed(pos);
	if (l == NULL) {
		return NULL;
	}
	pos += l->len;
	struct tnode *r = thompson_rest(pos);
	if (r == NULL) {
		return NULL;
	}
	pos += r->len;

	struct tnode *this = tnode_create(NT_REST);
	this->left = l;
	this->right = r;
	this->len = pos - input;

	int outlen = strlen(l->output) + strlen(r->output) + 1;
	this->output = realloc(this->output, sizeof(char) * outlen + 1);
	snprintf(this->output, outlen, "·%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_concat(char* input)
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

	int outlen = strlen(l->output) + strlen(r->output);
	this->output = (char*) realloc(this->output,
		sizeof(char) * (outlen + 1));
	snprintf(this->output, outlen, "%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_union(char* input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_UNION_EMPTY);
	}

	if (input[0] != '|') {
		fprintf(stderr, "nonempty unions must start with '|'");
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

	int outlen = strlen(l->output) + strlen(r->output) + 1; // '|'
	this->output = (char*) realloc(this->output,
		sizeof(char) * (outlen + 1));
	snprintf(this->output, outlen, "|%s%s", l->output, r->output);
	return this;
}


struct tnode*
thompson_parse(char* input)
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

	int outlen = strlen(l->output) + strlen(r->output);
	this->output = (char*) realloc(this->output,
		sizeof(char) * (outlen + 1));
	snprintf(this->output, outlen, "%s%s", l->output, r->output);
	return this;
}


struct tnode*
tnode_create(enum tnode_type type)
{
	struct tnode *this = (struct tnode*) malloc(sizeof(struct tnode));
	this->type = type;
	this->len = 0;
	this->output = (char*) malloc(sizeof(char));
	this->output = "";
	return this;
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
	free(this);
}
