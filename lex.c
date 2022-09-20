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

/* write_file: write value to file (and create if nonexistent) */
void 
write_file(char *path, char *value) 
{
    FILE *f = fopen(path, "w");
    if (f == NULL) {
        perror("error writing to file");
        exit(1);
    }
    fprintf(f, "%s", value);
    fclose(f);
}

void
declarations (char *file)
{
	/* ignore ws until %{ */
	for (; *file != '\0'; file++) {
		if (*file == '%') {
			file++;
			break;
		}
	}
	if (*file != '{') {
		// failure
	}
}


int 
main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "must provide input as string\n");
		return 1;
	}
	char* file = read_file(argv[1]);
	free(file);
}
