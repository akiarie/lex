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
seekdeclr(char *in)
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
decl(char *in, FILE *out)
{
	char *name = in;
	in++; /* first char must be letter */
	while (isletter(in[0])) {
		in++;
	}
}

void
declproper(char *in, FILE *out)
{
	while (true) {
		seekdeclr(in);
		decl(in, out);
	}
}


void 
declarations(char *in, FILE *out)
{
	if (in[0] != '%' || in[1] != '{') {
		fprintf(stderr, "declaration section must begin with constants");
		exit(1);
	}
	in += 2;
	while (in[0] != '\0') {
		if (in[0] == '%' && in[1] == '}') {
			in += 2;
			declproper(in, out);
			return;
		}
		fprintf(out, "%c", in[0]);
		in++;
	}
	fprintf(stderr, "declaration section ended without close of constants");
	exit(1);

}

void
transform(char *in, FILE *out)
{
	declarations(in, out);
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
        perror("error writing to file");
        exit(1);
    }

	transform(in, out);

    fclose(out);
	free(in);
}
