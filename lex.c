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
decl_seek(char *in)
{
	bool newline = false;
	while (true) {
		switch (in[0]) {
			case '\0':
				fprintf(stderr, "file ended in declarations section");
				exit(1);
			case '\n':
				newline = true;
				in++;
				continue;
			case '\t': case ' ':
				in++;
				continue;
		}
		if (isletter(in[0])) {
			if (!newline) {
				fprintf(stderr, "declaration cannot come before newline");
				exit(1);
			}
			return true;
		}
		fprintf(stderr, "unknown char '%c' in declaration section", in[0]);
		exit(1);
	}
}

void
decl_pattern_trans(char *in, FILE *out)
{
	char *name = in;
	in++; /* first char must be letter */
	while (isletter(in[0])) {
		in++;
	}
}

void
decl_proper_trans(char *in, FILE *out)
{
	while (true) {
		decl_seek(in);
		decl_pattern_trans(in, out);
	}
}

bool
decl_constants_atstart(char *in)
{
	return in[0] == '%' && in[1] == '{';
}


bool
decl_constants_atend(char *in)
{
	return in[0] == '%' && in[1] == '}';
}

void
decl_constants_trans(char *in, FILE *out)
{
	if (!decl_constants_atstart(in)) {
		fprintf(stderr, "constants section beginning with '%c%c'",
			in[0], in[1]);
		exit(1);
	}
	in += 2; // '%{'
	while (in[0] != '\0') {
		if (in[0] == '\0') {
			fprintf(stderr, "constants section not closed");
			exit(1);
		}
		if (decl_constants_atend(in)) {
			in += 2; // '%}'
			return;
		}
		fprintf(out, "%c", in[0]);
		in++;
	}
}

void
decl_trans(char *in, FILE *out)
{
	decl_constants_trans(in, out);
	decl_proper_trans(in, out);
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
		exit(1);
	}
	char* in = read_file(argv[1]);
	FILE *out = fopen(OUTPUT_FILE, "w");
	if (out == NULL) {
		fprintf(stderr, "error writing to file\n");
		exit(1);
	}
	transform(in, out);
	fclose(out);
	free(in);
}
