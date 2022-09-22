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
 *		| symbol - symbol
 *		| ε
 *
 *	symbol	→ a-Z | A-Z | 0-9
*/

enum tnode_type {
	NT_EXPR			= 1 << 0,
	NT_UNION		= 1 << 1,
	NT_UNION_EMPTY		= 1 << 2,

	NT_CONCAT		= 1 << 3,
	NT_REST			= 1 << 4,
	NT_REST_EMPTY		= 1 << 5,

	NT_CLOSED_STAR		= 1 << 6,
	NT_CLOSED_PLUS		= 1 << 7,
	NT_CLOSED		= 1 << 8,

	NT_BASIC_PAREN		= 1 << 9,
	NT_BASIC_CLASS		= 1 << 10,
	NT_BASIC_SYMBOL		= 1 << 11,
	NT_BASIC_EMPTY		= 1 << 12,

	NT_CLASS		= 1 << 13,
	NT_CLASS_INV		= 1 << 14,
	NT_INCLASS_SYMBOL	= 1 << 15,
	NT_INCLASS_CONCAT	= 1 << 16, // symbol inclass
	NT_INCLASS_RANGE	= 1 << 17,
	NT_INCLASS_EMPTY	= 1 << 18,

	NT_SYMBOL		= 1 << 19,

	NT_EMPTY		= 1 << 20, // ε
};

struct tnode {
	enum tnode_type type;
	char c;
	struct tnode *left; // also functions as default child
	struct tnode *right;
};

#endif