#ifndef LEX_GEN
#define LEX_GEN
#include<stdbool.h>

void
gen(struct token *tokens, int len, bool preamble, FILE *out);

#endif
