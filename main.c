#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
runlex(char *inputpath, char *outputpath)
{
	char* in = read_file(inputpath);
	FILE *out = fopen(outputpath, "w");
	if (out == NULL) {
		fprintf(stderr, "error writing to file\n");
		return 1;
	}
	struct lexer *lx = parse(in);
	gen(out, lx, true);
	lexer_destroy(lx);
	fclose(out);
	free(in);
	return 0;
}

int
main(int argc, char *argv[])
{
	char *output = OUTPUT_FILE;
	int opt;
	while ((opt = getopt(argc, argv, "o:")) != -1) {
		switch (opt) {
			case 'o':
				output = optarg;
				break;
			default:
				fprintf(stderr, "Usage: %s [-o output] lex.l\n", argv[0]);
				exit(1);
		}
	}
	if (optind >= argc) {
		fprintf(stderr, "must provide input as string\n");
		exit(1);
	}
	return runlex(argv[optind], output);
}
