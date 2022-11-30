#include<stdlib.h>

#include "selflex.h"

int
main()
{
	struct terminal *terminals = NULL;
	int len = 0;
	lex_string(&terminals, &len,
"%{\n"
"\n"
"#include<stdio.h>\n"
"\n"
"void printyy();\n"
"\n"
"%}\n");
	for (int i = 0; i < len; i++) {
		terminal_print(&terminals[i]);
		terminal_destroy(&terminals[i]);
	}
}
