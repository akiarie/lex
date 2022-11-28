#define YY_EOF 0

int
yylex()
{
	struct findresult *r = fsmlist_findnext(l, yyin);
	return YY_EOF;
}
