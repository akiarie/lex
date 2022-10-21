#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"


int
main()
{
	struct fsm* nfa = automata_string_conv("ab|ac");
	printf("\n");
	printf("FIRST\n");
	fsm_print(nfa, 0);
	nfa = fsm_sim(nfa, 'a');
	if (nfa != NULL) {
		printf("SECOND\n");
		fsm_print(nfa, 0);
		nfa = fsm_sim(nfa, 'c');
		if (nfa != NULL) {
			printf("LAST:\n");
			fsm_print(nfa, 0);
		}
	}
}
