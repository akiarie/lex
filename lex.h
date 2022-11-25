#ifndef LEX_LEX
#define LEX_LEX
#include<stdbool.h>

/* automata */
struct fsm;
struct fsmlist;

/* parser */
struct lexfile;

struct fsm *
lex_fsm_fromstring(char *regex, struct fsmlist *);

struct lexer;

/* lexer_fromfile: returns a lexer. takes ownership of the lexfile. */
struct lexer *
lexer_fromfile(struct lexfile *);

void
lexer_destroy(struct lexer *);

#endif
