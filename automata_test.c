#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"

#define LEN(a) (sizeof(a) / sizeof((a)[0]))

void
run()
{
	struct fsm* aut = automata_string_conv("a(b|c)*d");
	char *cases[] = {
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
	}
}

typedef void (*testcase)(void);

int
main()
{
	testcase cases[] = {
		run,
	};
	for (int i = 0, len = LEN(cases); i < len; i++) {
		cases[i]();
	}
}
