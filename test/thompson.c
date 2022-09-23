#include<stdio.h>
#include<stdlib.h>

#include "../thompson.h"

int
main()
{
	struct tnode *n = thompson_parse("a(b|c)*d");
	printf("%s\n", n->output);
	tnode_destroy(n);
}
