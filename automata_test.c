#include<stdio.h>
#include<stdlib.h>
#include<strings.h>

#include "automata.h"


int
main()
{
	struct fsm* nfa = automata_string_conv("ac|ad");
	printf("\n");
	fsm_print(nfa, 0);
	nfa = fsm_sim(nfa, 'a');
	if (nfa != NULL) {
		fsm_print(nfa, 0);
		/*nfa = fsm_sim(nfa, 'b');*/
		/*if (nfa != NULL) {*/
			/*fsm_print(nfa, 0);*/
		/*}*/
	}
}
