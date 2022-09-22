#ifndef LEX_THOMPSON
#define LEX_THOMPSON

/*
 * We make use of the following scheme to parse expressions:
 *	expr	→ concat union
 *	union	→ '|' concat union
 *		| ε
 *
 *	concat	→ closed rest
 *	rest	→ closed rest
 *		| ε
 *
 *	closed	→ basic * | basic + | basic
 *
 *	basic	→ ( expr )
 *		| class
 *		| symbol
 *		| ε
 *
 *	class	→ [ inclass ] | [ ^ inclass ]
 *	inclass	→ symbol
 *		| symbol inclass
 *		|
 *
 *	symbol	→ a-Z | A-Z | 0-9
*/

#endif
