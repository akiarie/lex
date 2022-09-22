#include<stdio.h>
#include<stdlib.h>

#include "../thompson.h"

int
main()
{
	struct tnode *n = thompson_parse("a(b|c)*d");
	free(n);
}
