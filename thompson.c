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
	char *pos = input;
	char c = pos[0];
	pos++;
	if (c == '\\') {
		char d = pos[0];
		switch (d) {
		case 'n':
			c = '\n';
			break;
		case 't':
			c = '\t';
			break;
		default:
			fprintf(stderr, "invalid control sequence '\\%c'", d);
			exit(1);
		}
		pos++;
	}
	if (isalpha(c) || isdigit(c) || isspace(c)) {
		struct tnode *this = tnode_create(NT_SYMBOL);
		this->value = (char *) malloc(sizeof(char) * 2);
		snprintf(this->value, 2, "%c", c);
		this->len = pos - input;
		return this;
	}
	return NULL;
}

struct tnode*
thompson_id(char *input)
{
	char *pos = input;
	if (!(isalpha(pos[0]) || pos[0] == '_')) {
		fprintf(stderr, "id must begin with letter or underscore");
		exit(1);
	}
	for (; pos[0] != '}'; pos++) {
		if (!(isalpha(pos[0]) || isdigit(pos[0]) || pos[0] == '_')) {
			fprintf(stderr, "invalid id char '%c'", pos[0]);
			exit(1);
		}
	}
	struct tnode *this = tnode_create(NT_ID);
	this->len = pos - input;
	int outlen = this->len + 1;
	this->value = (char *) malloc(sizeof(char) * outlen);
	snprintf(this->value, outlen, "%s", pos - this->len);
	return this;
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

	thompson_validate_range(l->value[0], r->value[0]);

	struct tnode *this = tnode_create(NT_RANGE);
	this->len = pos - input;
	this->left = l;
	this->right = r;

	int outlen = strlen(l->value) + strlen(r->value) + 1 + 1; // '-'
	this->value = (char *) realloc(this->value, sizeof(char) * outlen);
	snprintf(this->value, outlen, "%s-%s", l->value, r->value);
	return this;
}

struct tnode*
thompson_inclass(char *input)
{
	char *pos = input;
	if (pos[0] == ']') { // ε
		struct tnode *empty = tnode_create(NT_INCLASS);
		// important for concatenation below
		empty->value = (char *) calloc(1, sizeof(char));
		*empty->value = '\0';
		return empty;
	}

	struct tnode *l = thompson_atom(pos);
	pos += l->len;
	struct tnode *r = thompson_inclass(pos);
	pos += r->len;

	struct tnode *this = tnode_create(NT_INCLASS);
	this->len = pos - input;
	this->left = l;
	this->right = r;
	int outlen = strlen(l->value) + strlen(r->value) + 1;
	this->value = (char *) malloc(sizeof(char) * outlen);
	snprintf(this->value, outlen, "%s%s", l->value, r->value);
	return this;
}

struct tnode*
thompson_invclass(char *input)
{
	struct tnode *this = tnode_create(NT_CLASS);
	char *invsym = "";
	char *pos = input;
	if (pos[0] == '^') {
		this->value = (char *) malloc(sizeof(char) * 2);
		snprintf(this->value, 2, "^");
		invsym = "^";
		pos++;
	}
	struct tnode *icl = thompson_inclass(pos);
	pos += icl->len;
	this->left = icl;
	this->len = pos - input;

	int outlen = strlen(invsym) + strlen(icl->value) + 1;
	this->value = (char *) malloc(sizeof(char) * outlen);
	snprintf(this->value, outlen, "%s%s", invsym, icl->value);
	return this;
}

struct tnode*
thompson_class(char *input)
{
	struct tnode *this = tnode_create(NT_CLASS);
	char *invsym = "";
	char *pos = input;
	if (pos[0] == '^') {
		this->value = (char *) malloc(sizeof(char) * 2);
		snprintf(this->value, 2, "^");
		invsym = "^";
		pos++;
	}
	struct tnode *icl = thompson_inclass(pos);
	pos += icl->len;
	this->left = icl;
	this->len = pos - input;

	int outlen = strlen(invsym) + strlen(icl->value) + 1;
	this->value = (char *) malloc(sizeof(char) * outlen);
	snprintf(this->value, outlen, "%s%s", invsym, icl->value);
	return this;
}

typedef struct tnode * (*thompson_parser_func)(char *);

struct tnode*
thompson_bracketed(enum tnode_type type, thompson_parser_func func, char *input)
{
	char *brackets;
	switch (type) {
	case NT_BASIC_EXPR:
		brackets = "()";
		break;
	case NT_BASIC_CLASS:
		brackets = "[]";
		break;
	case NT_BASIC_ID:
		brackets = "{}";
		break;
	default:
		fprintf(stderr, "invalid bracket type '%d'", type);
		exit(1);
	}
	char *pos = input + 1;
	struct tnode *this = func(pos);
	this->type = type;
	pos += this->len;
	if (pos[0] != brackets[1]) {
		fprintf(stderr, "can't find closing bracket '%c'", brackets[1]);
		exit(1);
	}
	pos++;
	this->len = pos - input;
	return this;
}

struct tnode*
thompson_basic(char *pos)
{
	switch (pos[0]) {
	case '(':
		return thompson_bracketed(NT_BASIC_EXPR, thompson_parse, pos);
	case '[':
		return thompson_bracketed(NT_BASIC_CLASS, thompson_class, pos);
	case '{':
		return thompson_bracketed(NT_BASIC_ID, thompson_id, pos);
	}
	return thompson_symbol(pos);
}

bool
isclosure(char c) { return c == '*' || c == '+' || c == '?'; }

struct tnode*
thompson_closed(char *input)
{
	char *pos = input;
	struct tnode *basic = thompson_basic(pos);
	if (basic == NULL) {
		return NULL;
	}
	pos += basic->len;
	char *value = NULL;
	enum tnode_type type = NT_CLOSED_BLANK;
	if (!thompson_atend(pos)) {
		char c = pos[0];
		if (isclosure(c)) {
			type = NT_CLOSURE;
			pos += 1;
			if (!thompson_atend(pos) && (isclosure(pos[0]))) {
				fprintf(stderr, "double closures not allowed\n");
				exit(1);
			}
			int len = strlen(basic->value) + 1 + 1;
			value = (char *) malloc(sizeof(char) * len);
			snprintf(value, 2, "%c", c);
		}
	}
	struct tnode *this = tnode_create(type);
	this->value = value;
	this->left = basic;
	this->len = pos - input;
	return this;
}


struct tnode*
thompson_rest(char *input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_EMPTY);
	}

	char *pos = input;
	struct tnode *l = thompson_closed(pos);
	if (l == NULL) {
		return tnode_create(NT_EMPTY);
	}
	pos += l->len;
	struct tnode *r = thompson_rest(pos);
	if (r == NULL) {
		return tnode_create(NT_EMPTY);
	}
	pos += r->len;

	struct tnode *this = tnode_create(NT_REST);
	this->left = l;
	this->right = r;
	this->len = pos - input;
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
	return this;
}


struct tnode*
thompson_union(char *input)
{
	if (thompson_atend(input)) { // ε
		return tnode_create(NT_EMPTY);
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
	this->value = tnode_output(this);
	return this;
}


struct tnode*
tnode_create(enum tnode_type type)
{
	struct tnode *this = (struct tnode *) malloc(sizeof(struct tnode));
	this->left = this->right = NULL;
	this->type = type;
	this->len = 0;
	this->value = NULL;
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
	if (this->value != NULL) {
		free(this->value);
	}
}

struct tnode*
tnode_copy(struct tnode *this)
{
	struct tnode *new = tnode_create(this->type);
	if (this->left != NULL) {
		new->left = tnode_copy(this->left);
	}
	if (this->right != NULL) {
		new->right = tnode_copy(this->right);
	}
	if (this->value != NULL) {
		int len = strlen(this->value) + 1;
		new->value = (char *) malloc(sizeof(char) * len);
		snprintf(new->value, len, "%s", this->value);
	}
	new->len = this->len;
	return new;
}

char*
tnode_output(struct tnode *this)
{
	char *output;
	char *l;
	char *r;
	int len;
	switch (this->type) {
	case NT_EXPR: case NT_CONCAT:
		l = tnode_output(this->left);
		r = tnode_output(this->right);
		len = strlen(l) + strlen(r) + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "%s%s", l, r);
		free(l);
		free(r);
		break;

	case NT_UNION:
		l = tnode_output(this->left);
		r = tnode_output(this->right);
		len = strlen(l) + strlen(r) + 1 + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "|%s%s", l, r);
		free(l);
		free(r);
		break;
	case NT_REST:
		l = tnode_output(this->left);
		r = tnode_output(this->right);
		len = strlen(l) + strlen(r) + 1 + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, ".%s%s", l, r);
		free(l);
		free(r);
		break;

	case NT_CLOSED_BLANK:
		l = tnode_output(this->left);
		len = strlen(l) + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "%s", l);
		free(l);
		break;

	case NT_CLOSURE:
		l = tnode_output(this->left);
		len = strlen(l) + strlen(this->value) + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "%s%s", l, this->value);
		free(l);
		break;

	case NT_BASIC_EXPR:
		len = strlen(this->value) + 2 + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "(%s)", this->value);
		break;
	case NT_BASIC_CLASS:
		len = strlen(this->value) + 2 + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "[%s]", this->value);
		break;
	case NT_BASIC_ID:
		len = strlen(this->value) + 2 + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "{%s}", this->value);
		break;

	case NT_SYMBOL:
		len = strlen(this->value) + 1;
		output = (char *) malloc(sizeof(char) * len);
		snprintf(output, len, "%s", this->value);
		break;

	case NT_EMPTY:
		output = (char *) malloc(sizeof(char));
		*output = '\0';
		break;

	default:
		fprintf(stderr, "cannot output type %d\n", this->type);
		exit(1);
	}
	return output;
}

char*
thompson_prepstring(char *stack)
{
	int len = strlen(stack) + 1;
	char *heap = (char *) malloc(sizeof(char) * len);
	snprintf(heap, len, "%s", stack);
	return heap;
}

char*
tnode_type_string(enum tnode_type type)
{
	switch (type) {
	case NT_EXPR:
		return thompson_prepstring("expr");
	case NT_CONCAT:
		return thompson_prepstring("concat");
	case NT_UNION:
		return thompson_prepstring("union");
	case NT_REST:
		return thompson_prepstring("rest");
	case NT_CLOSED_BLANK:
		return thompson_prepstring("closed blank");
	case NT_CLOSURE:
		return thompson_prepstring("closure");
	case NT_BASIC_EXPR:
		return thompson_prepstring("basic expr");
	case NT_BASIC_CLASS:
		return thompson_prepstring("basic class");
	case NT_BASIC_ID:
		return thompson_prepstring("basic id");
	case NT_SYMBOL:
		return thompson_prepstring("symbol");
	case NT_EMPTY:
		return thompson_prepstring("empty");
	default:
		fprintf(stderr, "unknown type %d\n", type);
		exit(1);
	}
}


void
thompson_indent(int len, char c)
{
	for (int i = 0; i < len; i++) {
		putchar(c);
	}
}

void
tnode_print(struct tnode *this, int level)
{
	thompson_indent((level-1) * 8, ' ');
	if (level > 0) {
		thompson_indent(7, '-');
		putchar('>');
	}
	char *type;
	switch (this->type) {
		case NT_SYMBOL:
			printf("'%s'", this->value);
			break;
		case NT_CLOSURE:
			printf("%s", this->value);
			break;
		case NT_EMPTY:
			printf("ε");
			break;
		default:
			type = tnode_type_string(this->type);
			printf("[%s]", type);
			free(type);
			break;
	}
	printf("\n");
	if (this->left != NULL) {
		thompson_indent(level * 8, ' ');
		printf("|\n");
		tnode_print(this->left, level+1);
	}
	if (this->right != NULL) {
		thompson_indent(level * 8, ' ');
		printf("|\n");
		tnode_print(this->right, level+1);
	}
}

