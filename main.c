#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "parse.h"
#include "gen.h"

#define OUTPUT_FILE "lex.yy.c"

/* read_file: reads contents of file and returns them
 * caller must free returned string
 * see https://stackoverflow.com/a/14002993 */
char *
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
	struct lexer *lx = parse(in);
	gen(out, lx, true);
	lexer_destroy(lx);
	fclose(out);
	free(in);
}
