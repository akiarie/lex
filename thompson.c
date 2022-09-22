#include<stdio.h>
#include<stdlib.h>

#include "thompson.h"

bool
thompson_atend(char* input)
{
	return input[0] == '\0'; // TODO: add support for parentheses
}


struct tnode*
thompson_basic(char* input)
{
	fprintf(stderr, "basic not implemented");
	exit(1);
}


struct tnode*
thompson_closed(char* input)
{
	struct tnode *this = talloc();
	this->type = NT_CLOSED;
	char *pos = input;
	this->left = thompson_basic(pos);
	pos += this->left->len;
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
	struct tnode *this = talloc();
	if (thompson_atend(input)) { // ε
		this->type = NT_REST_EMPTY;
		return this;
	}
	this->type = NT_REST;
	char *pos = input;
	this->left = thompson_closed(pos);
	pos += this->left->len;
	this->right = thompson_rest(pos);
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_concat(char* input)
{
	struct tnode *this = talloc();
	this->type = NT_CONCAT;
	char *pos = input;
	this->left = thompson_closed(pos);
	pos += this->left->len;
	this->right = thompson_rest(pos);
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_union(char* input)
{
	struct tnode *this = talloc();
	if (thompson_atend(input)) { // ε
		this->type = NT_UNION_EMPTY;
		return this;
	}
	this->type = NT_UNION;
	if (input[0] != '|') {
		fprintf(stderr, "nonempty unions must start with '|'");
		exit(1);
	}
	printf("|");
	char *pos = input + 1;
	this->left = thompson_concat(pos);
	pos += this->left->len;
	this->right = thompson_union(pos);
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_parse(char* input)
{
	struct tnode *this = talloc();
	this->type = NT_EXPR;
	char *pos = input;
	this->left = thompson_concat(pos);
	pos += this->left->len;
	this->right = thompson_union(pos);
	this->len = pos - input;
	return this;
}


struct tnode*
talloc()
{
	struct tnode *n = (struct tnode*) malloc(sizeof(struct tnode));
	n->len = 0;
	return n;
}
