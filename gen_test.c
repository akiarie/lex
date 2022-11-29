#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"
#include "parse.h"
#include "gen.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

static char *
dynamic_name(char *static_name)
{
	int len = strlen(static_name) + 1;
	char *name = (char *) malloc(sizeof(char) * len);
	snprintf(name, len, "%s", static_name);
	return name;
}

void
run()
{
	struct pattern patterns[] = {
		{"float", "float"},
		{"other", "[a-zA-Z0-9 \\n]"},
	};
	struct token tokens[] = {
		{false,	"float",	"printf(\"double\");"},
		{false,	"other",	"printyy();"},
	};
	struct lexer *lx = lexer_create(
		dynamic_name(
"\n"
"#include<stdio.h>\n"
"\n"
"void printyy();\n"
"\n"
"\n"),
		dynamic_name(
"\n"
"void printyy()\n"
"{\n"
"	printf(\"%.*s\", (int) yyleng, yytext);\n"
"}\n"
"\n"
"/* read_file: reads contents of file and returns them\n"
" * caller must free returned string \n"
" * see https://stackoverflow.com/a/14002993 */\n"
"char* read_file(char *path)\n"
"{\n"
"    FILE *f = fopen(path, \"rb\");\n"
"    fseek(f, 0, SEEK_END);\n"
"    long fsize = ftell(f);\n"
"    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */\n"
"    char *str = malloc(fsize + 1);\n"
"    fread(str, fsize, 1, f);\n"
"    fclose(f);\n"
"    str[fsize] = '\\0';\n"
"    return str;\n"
"}\n"
"\n"
"int main(int argc, char* argv[])\n"
"{\n"
"	if (argc != 2) {\n"
"		perror(\"must supply input file\");\n"
"		exit(1);\n"
"	}\n"
"	char *infile = read_file(argv[1]);\n"
"	yy_scan_string(infile);\n"
"	free(infile);\n"
"	yylex(); \n"
"}\n"),
		patterns, LEN(patterns), tokens, LEN(tokens));
	gen(stdout, lx, true);
	lexer_destroy(lx);
}

typedef void (*test)(void);

int
main()
{
	test tests[] = {
		run,
	};
	for (int i = 0, len = LEN(tests); i < len; i++) {
		tests[i]();
	}
}
