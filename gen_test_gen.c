/* BEGIN */

/* BEGIN preamble */

#include<stdio.h>

void printyy();


/* END preamble */
/* BEGIN lexer */
char *yyin = NULL;

void
yy_scan_string(char *s)
{
	int len = strlen(s) + 1;
	yyin = (char *) malloc(sizeof(char) * len);
	snprintf(yyin, len, "%s", s);
}

static struct fsm *
gettokenfsm(struct token *t, struct fsmlist *l)
{
	if (t->literal) {
		/* TODO: create token */
		fprintf(stderr, "literal tokens NOT IMPLEMENTED\n");
		exit(1);
	}
	struct fsm *s = fsmlist_findfsm(l, t->name);
	if (NULL == s) {
		fprintf(stderr, "cannot find pattern for '%s'\n", t->name);
		exit(1);
	}
	return fsm_copy(s);
}

struct fsmlist *yyfsmlist = NULL;
unsigned long yyleng = 0;
char *yytext = NULL;

static void
yyfsmlistprep()
{
	struct pattern p[] = {
		{"float",	(char []){102,108,111,97,116,0} /* float */},
		{"other",	(char []){91,97,45,122,65,45,90,48,45,57,32,92,110,93,0} /* [a-zA-Z0-9 \n] */},
	};
	struct fsmlist *l = NULL;
	for (int i = 0; i < 2; i++) {
		struct fsm *s = fsm_fromstring(p[i].pattern, l);
		l = fsmlist_append(l, p[i].name, s);
	};
	struct token t[] = {
		{false,	"float",	(char []){112,114,105,110,116,102,40,34,100,111,117,98,108,101,34,41,59,0}},
		{false,	"other",	(char []){112,114,105,110,116,121,121,40,41,59,0}},
	};
	for (int i = 0; i < 2; i++) {
		yyfsmlist = fsmlist_append(yyfsmlist, t[i].name, gettokenfsm(&t[i], l));
	};
	fsmlist_destroy(l);
}

int
yylex()
{
	if (NULL == yyfsmlist) {
		yyfsmlistprep();
	}
	struct findresult *r = fsmlist_findnext(yyfsmlist, yyin);
	yyleng = r->len; yytext = yyin;
	yyin += yyleng;
	if (yyleng == 0) {
		assert(*yyin == '\0');
		return 0;
	}
	if (r->fsm == NULL) {
		fprintf(stderr, "unmatched '%.*s'\n", (int) yyleng, yytext);
		exit(1);
	}
	/* ⊢ r->fsm != NULL && yyleng > 0 */
	if (strcmp(r->fsm, "float") == 0) {
		printf("double");
	} else if (strcmp(r->fsm, "other") == 0) {
		printyy();
	}
	/* recurse if no return-action above */
	return yylex();
}
/* END lexer */
/* BEGIN postamble */

void printyy()
{
	printf("%.*s", (int) yyleng, yytext);
}

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

int main(int argc, char* argv[])
{
	if (argc != 2) {
		perror("must supply input file");
		exit(1);
	}
	char *infile = read_file(argv[1]);
	yy_scan_string(infile);
	free(infile);
	yylex();
}

/* END postamble */

/* END */
