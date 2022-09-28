#ifndef LEX_AUTOMATA
#define LEX_AUTOMATA

struct nfa {
};

struct nfa*
automata_compute(char *);

struct nfa*
automata_concat(struct nfa *, struct nfa *);

struct nfa*
automata_or(struct nfa *, struct nfa *);

struct nfa*
automata_closure(struct nfa *, char closure);

#endif
