#define YY_EOF 0

int
yylex()
{
	if (NULL == yyfsmlist) {
		yyfsmlistprep();
	}
	struct findresult *r = fsmlist_findnext(yyfsmlist, yyin);
	return YY_EOF;
}
