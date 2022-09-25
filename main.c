/*
 * A Lex program has the following form:
 * 		declarations
 * 		%%
 * 		translation rules
 * 		%%
 * 		auxiliary functions
 *
 * In the declarations section anything in the initial %{ }% is copied directly
 * to the output file, as well as the whole auxiliary functions section.
 *
 * The declarations section then contains a list of regular definitions, and in
 * the translation rules section these definitions (together with plaintext
 * constants) are used in a pattern action sequence:
 * 		pattern 	{ action }
 * where the action is executed, a return value in the action indicating that a
 * token has been found (by returning the manifest constant denoting the token).
 * It is the responsibility of the action to set the global variable yylval to
 * the value of the token.
 *
 * The functions in the auxiliary function section expect to find the lexeme
 * using the global variables yytext (pointer to the first character) and yyleng
 * (length).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "automata.h"

#define OUTPUT_FILE "lex.yy.c"

/* read_file: reads contents of file and returns them
 * caller must free returned string
 * see https://stackoverflow.com/a/14002993 */
char*
read_file(char *path)
{
	FILE *f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
	char *str = malloc(fsize + 1);
	fread(str, fsize, 1, f);
	fclose(f);
	str[fsize] = '\0';
	return str;
}

bool
isletter(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool
istaborspace(char c)
{
	return c == ' ' || c == '\t';
}

bool
decl_atend(char *in)
{
	return in[0] == '%' && in[1] == '%';
}

int
decl_pattern_trans(char *in, FILE *out)
{
	char *pos;
	fprintf(out, ", pattern: ");
	for (pos = in; pos[0] != '\n'; pos++) {
		fprintf(out, "%c", pos[0]);
	}
	fprintf(out, "\n");
	return pos - in;
}

int
decl_pattern_seek(char *in)
{
	char *pos;
	for (pos = in; istaborspace(pos[0]); pos++) {}
	return pos - in;
}

int
decl_id_trans(char *in, FILE *out)
{
	fprintf(out, "id: ");
	char *pos;
	for (pos = in; isletter(pos[0]); pos++) {
		fprintf(out, "%c", pos[0]);
	}
	return pos - in;
}

int
decl_decl_trans(char *in, FILE *out)
{
	char *pos = in;
	pos += decl_id_trans(pos, out);
	pos += decl_pattern_seek(pos);
	pos += decl_pattern_trans(pos, out);
	exit(1);
}

int
decl_seeknewline(char *in)
{
	char *pos;
	for (pos = in; pos[0] != '\n'; pos++) {
		switch (pos[0]) {
			case '\0':
				fprintf(stderr, "ended in declarations");
				exit(1);
			case '\t': case ' ':
				continue;
		}
		fprintf(stderr, "unknown char '%c' while seeking for newline",
			pos[0]);
		exit(1);
	}
	return pos - in;
}

struct seek_result {
	bool success;
	int delta;
};

struct seek_result
decl_seek(char *in)
{
	char *pos = in;
	pos += decl_seeknewline(pos); // newline must precede declaration
	while (!isletter(pos[0])) {
		if (decl_atend(in)) {
			return (struct seek_result){false, pos - in};
		}
		switch (pos[0]) {
			case '\0':
				fprintf(stderr, "ended in declarations");
				exit(1);
			case '\n': case '\t': case ' ':
				pos++;
				continue;
		}
		fprintf(stderr, "unknown char '%s' in declarations", in);
		exit(1);
	}
	return (struct seek_result){true, pos - in};
}

int
decl_proper_trans(char *in, FILE *out)
{
	char *pos = in;
	struct seek_result r;
	for (r = decl_seek(pos); r.success; r = decl_seek(pos)) {
		pos += r.delta;
		pos += decl_decl_trans(pos, out);
	}
	return pos - in;
}

bool
decl_cons_atstart(char *in)
{
	return in[0] == '%' && in[1] == '{';
}


bool
decl_cons_atend(char *in)
{
	return in[0] == '%' && in[1] == '}';
}

int
decl_cons_proper_trans(char *in, FILE *out)
{
	char *pos;
	for (pos = in; !decl_cons_atend(pos); pos++) {
		if (pos[0] == '\0') {
			fprintf(stderr, "ended while outputting constants");
			exit(1);
		}
		fprintf(out, "%c", pos[0]);
	}
	return pos - in;
}

int
decl_cons_trans(char *in, FILE *out)
{
	if (!decl_cons_atstart(in)) {
		fprintf(stderr, "constants section beginning with '%c%c'",
			in[0], in[1]);
		exit(1);
	}
	char *pos = in + 2; // '%{'
	pos += decl_cons_proper_trans(pos, out);
	pos += 2; // '%}'
	return pos - in;
}

int
decl_trans(char *in, FILE *out)
{
	char *pos = in;
	pos += decl_cons_trans(pos, out);
	pos += decl_proper_trans(pos, out);
	return pos - in;
}

void
transform(char *in, FILE *out)
{
	decl_trans(in, out);
}

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "must provide input as string\n");
		return 1;
	}
	char* in = read_file(argv[1]);
	FILE *out = fopen(OUTPUT_FILE, "w");
	if (out == NULL) {
		fprintf(stderr, "error writing to file\n");
		return 1;
	}
	transform(in, out);
	fclose(out);
	free(in);
}
