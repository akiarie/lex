#include<stdio.h>
#include<stdlib.h>

#include "thompson.h"

bool
thompson_atend(char* input)
{
	return input[0] == '\0'; // TODO: add support for parentheses
}

struct tnode*
thompson_closed(char* input)
{
	fprintf(stderr, "closed not implemented");
	exit(1);
}

struct tnode*
thompson_rest(char* input)
{
	fprintf(stderr, "rest not implemented");
	exit(1);
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
		this->type = NT_EMPTY;
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
	return (struct tnode*) malloc(sizeof(struct tnode));
}
