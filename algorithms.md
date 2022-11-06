# algorithms

## header
```C
/* forward declarations */

struct fsm;

π
fsm_defined(struct fsm *);
```

```C
/* edge */

struct edge {
	char c;
	struct fsm *dest;
};

axiom π
edge_allocd(struct edge *);

π
edge_defined(struct edge *e):
	e != NULL && edge_allocd(e) && fsm_defined(e->dest)
		&& (e->c == '\0' || isletter(e->c));

π
edge_points(struct edge *e, struct fsm *to, char c):
	edge_defined(e) && e->dest == to && e->c == c;

π
edge_matchacc(struct edge *e, bool accepting):
	edge_defined(e) 
		&& ((e->c == '\0' && e->dest->accepting) ? accepting : true);
		/* (e->c == '\0' && e->dest->accepting) implies accepting */

π
edge_equal(struct edge *e1, struct edge *e2):
	edge_defined(e1) && edge_defined(e2)
		&& e1->c == e2->c
		&& e1->dest == e2->dest;
```


```C
/* fsm */

/* forward declarations */
struct fsmset;

π
fsmset_defined(struct fsmset *);


struct fsm {
	bool accepting;
	size_t nedges;
	struct edge **edges;
};

axiom π
fsm_allocd(struct fsm *);

π
fsm_defined(struct fsm *s):
	s != NULL && fsm_allocd(s) 
		&& s->edges != NULL && edge_defined(e) 
		for e := s->edges[i] for i in {i := 0; i < s->nedges; i++};

range (struct edge *)
operator "all" (π (struct edge *) P) (struct fsm *s):
	fsm_defined(s)
	? s->edges[i] for i in 
		all π(int i)(P(s->edges[i])) {i := 0; i < s->nedges; i++}
	: Ø;

π
operator (struct edge *e) "from" (struct fsm *s):
	fsm_defined(s) && (all π(struct edge *ee)(e == ee) s) != Ø;

π
fsm_edgeto(struct fsm *s, struct fsm *t, char c):
	fsm_defined(s)
		&& edge_matchacc(e, s->accepting)
			for struct edge e from s
		&& edge_points(e, t, c)
			for some struct edge *e from s;

π
fsm_npred(struct fsm *s, π(struct fsm *) P, int n):
	P(s) || e->c == '\0' && fsm_npred(e->dest, P, n-1) 
		for some struct edge *e from s;

π
fsm_edgeson(struct fsm *s, struct fsm *t, char c):
	fsm_defined(s) && fsm_npred(s, π(struct fsm *r) fsm_edgeto(r, t, c), n)
		for some n in {n := 0; ; n++};

π
fsm_reaches(struct fsm *s, struct fsm *t):
	s == t || fsm_edgeson(s, t, '\0');

thm
fsm_edgetransitivity(struct fsm *s, struct fsm *t, struct fsm *r, char c):
	!(fsm_reaches(s, t) && fsm_edgeson(t, r, c)) || fsm_edgeson(s, r, c);
/*
Suppose fsm_reaches(s, t) and fsm_edgeson(t, r, c).
Focusing on fsm_reaches(s, t), if s == t then clearly fsm_edgeson(s, r, c).
Otherwise, we have
	fsm_edgeson(s, t, '\0') && fsm_edgeson(t, r, c). (1)

Begin by defining
	Π(σ, τ, n):
	fsm_defined(σ) && fsm_npred(σ, π(struct fsm *u) fsm_edgeto(u, τ, '\0'), n).
and T(σ, τ, n) as
	Π(σ, τ, n) && fsm_edgeson(τ, r, c).
We shall show that T(σ, τ, n) implies fsm_edgeson(σ, r, c) for any
nonnegative integer n, struct fsm *σ, *τ.
	
Note that fsm_edgeson(s, t, '\0') is
	Π(s, t, n) for some n in {n := 0; ; n++},
so (1) implies T(s, t, n). 
Thus the implication from T(σ, τ, n) to fsm_edgeson(σ, r, c)
sufficient for the truth of the theorem.

Let T(σ, τ, 0) for some struct fsm *σ, *τ. Then Π(σ, τ, 0), and we have 
	fsm_defined(σ) && fsm_edgeto(σ, τ, '\0').	(2)
By definition, this implies
	edge_points(e, τ, '\0') for some struct edge *e from σ.	(*)
Now fsm_edgeson(σ, r, c) is
	(3)
	fsm_defined(σ) && fsm_npred(σ, π(struct fsm *u) fsm_edgeto(u, r, c), n)
		for some n in {n := 0; ; n++}.
Since the first term is implied by (2), the fsm_npred term will hold if
	fsm_edgeto(σ, r, c) || ( n > 0 && fsm_defined(σ)
		&& h->c == '\0' && fsm_npred(h->dest, P, n-1)
		for some struct edge *h from σ ).
If the first term is true, then fsm_edgeson(σ, r, c) with n = 0 in (3)
definition (and using the assumption fsm_defined(σ) from (2)).
Otherwise, the above formula will hold if we can find an integer n with
	h->c == '\0' && fsm_npred(h->dest, P, n-1)
		for some struct edge *h from σ,
because fsm_defined(σ) was assumed in assuming T(σ, τ, 0). With h := e in (*),
it is sufficient for
	fsm_npred(τ, P, k) for some n in {n := 0; ; n++}
to hold, which is exactly fsm_edgeson(τ, r, c).

Now assume T(σ, τ, N) implies fsm_edgeson(σ, r, c) for any nonnegative integer N
and struct fsm *σ, *τ and assume T(σ, τ, N+1).
This means we have Π(σ, τ, N+1), or fsm_defined(σ) and
	fsm_edgeto(σ, τ, '\0') || ( e->c == '\0'
		&& fsm_npred(e->dest, π(struct fsm *u) fsm_edgeto(u, τ, '\0'), N)
		for some struct edge *e from σ ).
If fsm_edgeto(σ, τ, '\0') then Π(σ, τ, 0) holds, which implies T(σ, τ, 0), which
we have already shown implies fsm_edgeson(σ, r, c).
Assuming otherwise, from the above, we must have
	e->c == '\0'
	&& fsm_npred(e->dest, π(struct fsm *u) fsm_edgeto(u, τ, '\0'), N)
		for some struct edge *e from σ.
Let struct fsm *e satisfy the above. Because (e from σ), we may derive
	edge_defined(e)
from fsm_defined(σ), and in turn fsm_defined(e->dest) from the definition of
edge_defined. But now
	fsm_defined(e->dest) 
	&& fsm_npred(e->dest, π(struct fsm *u) fsm_edgeto(u, τ, '\0'), N),
which is Π(e->dest, τ, N). But our assumption T(σ, τ, N+1) includes 
	fsm_edgeson(τ, r, c),
so we now have T(e->dest, τ, N), which establishes (by induction) 
	fsm_edgeson(e->dest, r, c).
This then gives us T(σ, e->dest, 0), because Π(σ, e->dest, 0) is fsm_defined(σ)
(which we know holds) and
	fsm_npred(σ, π(struct fsm *u) fsm_edgeto(u, e->dest, '\0'), 0),
which we know is true because fsm_edgeto(σ, e->dest, '\0').
We conclude from the proof of the N = 0 case above that
	fsm_edgeson(σ, r, c).
*/

inv
fsm_inv(struct fsm *s):
	{s, s->nedges} union e for e from s;

/*
pre:
post:
	- fsm_defined(ret)
	- ret->accepting == accepting
*/
struct fsm *
fsm_create(bool accepting);

/*
pre:
	- fsm_defined(s)
	- edge_points(e, e->dest, e->c)
	- realloc working
post:
	- fsm_defined(s)
	- fsm_edgeto(s, e->dest, e->c)
*/
void
fsm_addedge(struct fsm *s, struct edge *e);

/*
inv:	
	- fsm_inv(s)
pre:
	- fsm_defined(s)
	- c == '\0' || isletter(c)
post:
	- fsmset_defined(ret)
	- fsm_edgeson(s, t, c) for struct fsm *t in ret
	- !fsm_edgeson(s, t, c) || fsm_reaches(r, t) for some struct *r in ret
		for t := e->dest for struct edge *e from s;
*/
static struct fsmset *
fsm_move(struct fsm *s, char c);


/*
inv:
	- fsm_inv(s)
pre:
	- fsm_defined(s)
	- isletter(c)
post:
	- fsm_defined(ret)
	- fsm_edgeson(s, t, c)
		for t := e->dest for struct edge *e from ret
	- fsm_reaches(ret, t) || !fsm_edgeson(s, t, c)
		for t := e->dest for struct edge *e from s
*/
struct fsm *
fsm_sim(struct fsm *s, char c);
```

## implementation

### edge
```C
/*
pre:
	- malloc working && realloc working
	- fsm_defined(dest)
	- c == '\0' || isletter(c)
post:
	- edge_defined(ret)
	- ret->dest == dest && ret->c == c
*/
static struct edge *
edge_create(struct fsm *dest, char c);
```

### fsmset
```C
struct fsmset {
	struct fsm **arr;
	size_t len;
};

static range (struct fsm *)
operator "all" (π (struct fsm *) P) (struct fsmset *l):
	l->arr[i] for i in all π(int i) (P(l->arr[i])) {i := 0; i < l->len; i++};

static π
operator (struct fsm *s) "in" (struct fsmset *l):
	(all π(struct fsm *t) (s == t) l) != Ø;

axiom π
fsmset_allocd(struct fsmset *);

static π
fsmset_defined(struct fsmset *l):
	fsmset_allocd(l) && l != NULL && l->len >= 0
		&& fsm_defined(s) for struct fsm *s in l;

static π
fsmset_empty(struct fsmset *l):
	fsmset_defined(l) && l->len == 0
		&& (all π(struct fsm *s) (s == s) l) == Ø;

static inv
fsmset_inv(struct fsm *l): {l, l->len} union s for struct fsm *s in l;

/*
pre:
	- malloc working
post:
	- fsmset_allocd(ret)
	- fsmset_defined(ret)
	- fsmset_empty(ret)
*/
static struct fsmset *
fsmset_create()
{
	struct fsmset *l = (struct fsmset *) malloc(sizeof(struct fsmset));
	l->arr = NULL;
	l->len = 0;
	return l;
}
/*
The first postcondition is axiomatic.
By virtue of malloc working, we will have l != NULL after the
declaration-assignment. Moreover, we have l->len >= 0, clearly, and because 
	all π(struct fsm *s) (s == s) l
must be the empty set (because l->len == 0), it follows that fsmset_defined(l).
Finally, the previous implies fsmset_empty(l).
*/

/*
inv:
	- fsm_inv(s)
pre:
	- realloc working
	- fsmset_defined(l)
	- fsm_defined(s)
post:
	- s in l
	- t in l for struct fsm *t in pre(l)
	- (s == t || t in pre(l)) for struct fsm *t in l
*/
static void
fsmset_add(struct fsmset *l, struct fsm *s)
{
	l->arr = (struct fsm **) realloc(l->arr, sizeof(struct fsm) * ++l->len);
	l->arr[l->len-1] = s;
}
/*
Define the predicate
	P(n):
	1: t in l for struct fsm *t in pre(l)
	2: (s == t || t in pre(l))
		for t := l->arr[i] for i in {i := 0; i < n; i++}.
Evidently P.1(l->len) holds initially. 
If l->len == 0, then P.2(l->len) holds by the principle of expulsion. Otherwise,
we have l->len > 0 because of the precondition fsmset_defined(l).
Let t := l->arr[i] for some i in {i := 0; i < l->len; i++}. Because initially
	l == pre(l),
the (t in l) in P.1 implies (t in pre(l)), so
	P(l->len)
holds. This can be re-written as
	P((l->len+1)-1),
so the assignment ++l->len implies P(l->len-1).

Similar to the argument in the proof for fsm_addedge, the realloc will preserve
P(l->len-1) and ensure that l->arr points to a location in memory with size
	sizeof(struct fsmset) * l->len.
Thus the assignment
	l->arr[l->len-1] = s
is possible. It also implies 
	all π(struct fsm *t) (s == t) l
is not Ø, because
	s == l->arr[i] for i := l->len-1,
thus clearly P(l->len) is established.
This also implies (s in l).

The postconditions follow from (s in l) and P(l->len).
*/


/*
inv:
	- fsmset_inv(s)
pre:
	- fsmset_defined(l)
	- fsmset_defined(m)
post:
	- s in l for struct fsm *s in pre(l)
	- s in l for struct fsm *s in m
*/
static void
fsmset_addrange(struct fsmset *l, struct fsmset *m)
{
	for (int i = 0; i <= m->len; i++) {
		fsmset_add(l, m->arr[i]);
	}
}

/*
inv:
	- fsmset_inv(l)
pre:
	- fsmset_defined(l)
	- c == '\0' || isletter(c)
post:
	- fsmset_defined(ret)
	- fsm_edgeson(s, t, c) for some struct fsm *s in l
		for struct fsm *t in ret
	- !fsm_edgeson(s, t, c) || fsm_reaches(r, t) for some struct fsm *r in ret
		for t := e->dest for struct edge *e from s
		for struct fsm *s in l
*/
static struct fsmset *
fsmset_move(struct fsmset *l, char c)
{
	struct fsmset *next = fsmset_create();
	/* ⊢ fsmset_defined(next) && fsmset_empty(next) */
	for (int i = 0; i < l->len; i++) {
		/* ⊢ P(i) */
		struct fsm *s = l->arr[i];
		struct fsmset *m = fsm_move(s, c);
		/*
		⊢ fsmset_defined(m)
		⊢ fsm_edgeson(s, t, c) for struct fsm *t in m
		⊢ !fsm_edgeson(s, t, c)
			|| fsm_reaches(r, t) for some struct *r in m
			for t := e->dest for struct edge *e from s
		*/
		fsmset_addrange(l, m);
		/*
		⊢ P(i+1)
		*/
	}
	return next;
}
/*
If l->len == 0, the loop body never executes and the return value satisifies our
postconditions by default by the principle of explosion, because 
	all struct fsm *s in l
is the empty set, and similarly for next. 

Otherwise, we assume l->len > 0 and define the predicate
	π P(int n):
	0: fsmset_defined(next)
	1: fsm_edgeson(s, t, c) for some struct *s in l
		for struct fsm *t in next
	2: !fsm_edgeson(s, t, c) || fsm_reaches(r, t) for some struct *r in next
		for t := e->dest for struct edge *e from s
		for s := l->arr[j] for j in {j := 0; j < n; j++}. 

Upon initialisation of the loop, clearly fsmset_defined(next) holds, and P.1(i)
follow from the principle of explosion because fsmset_empty(next). Since i == 0,
P.2(i) == P.2(0), which again must hold by the principle of explosion. Thus
P(i) holds upon initialisation.

As shown above, under assumption of P(i), the interior body of the loop
(excluding the increment i++) has the effect of establishing P(i+1). But then by
the axiom of assignment the incremental assignment i++ will restore P(i).

The termination proof is identical to that for fsmset_tofsm, with the integer
function being t(i) := l->len - i.

Thus, when the loop terminates we will have P(l->len). P.1 and P.2 will imply
the first two postconditions. P.3 implies the third postcondition because the
range
	l->arr[j] for j in {j := 0; j < l->len; j++}
is equivalent to
	all struct fsm *s in l,
which is
	l->arr[i] for i in all π(int i) (Q(l->arr[i])) {i := 0; i < l->len; i++}
where we have
	π Q(struct fsm *s): (all π(struct fsm *t) (s == t) l) != Ø.
Since for any struct fsm *s = l->arr[i] for some i in {i := 0; i < l->len; i++}
we have s == t when struct fsm *t = l->arr[i], it follows that
	Q(l->arr[i])
holds in every case, thus (all struct fsm *s in l) is
	s := l->arr[j] for j in {j := 0; j < l->len; j++}.
*/

/*
inv: fsmset_inv(l)
pre: fsmset_defined(l)
post:
- fsm_reaches(s, t) for some struct fsm *s in l for struct fsm *t in ret
- r in ret for struct fsm *r in 
	all π(struct fsm *t) (t in l || fsm_reaches(s, t) for some struct *s in l) l;
*/ 
static struct fsmset *
fsmset_epsclosure(struct fsmset *l);

/*
inv:
	- fsmset_inv(l)
pre:
	- fsmset_defined(l)
post:
	- fsm_defined(ret)
	- fsm_reaches(ret, s)
		for struct fsm *s in l
	- e->c == '\0' && e->dest in l
		for struct edge *e from ret
*/
static struct fsm *
fsmset_tofsm(struct fsmset *l)
{
	struct fsm *s = fsm_create(false);
	for (int i = 0; i != l->len; i++) {
		if (l->arr[i]->accepting) {
			s->accepting = true;
		}
		fsm_addedge(s, edge_create(l->arr[i], '\0'));
	}
	return s;
}
/*
Consider the relation P:
	(1.)
	fsm_edgeto(s, t, '\0') && (!t->accepting || s->accepting)
		for t := l->arr[j] for j in {j := 0; j < i; j++}

	(2.) fsm_defined(s)

	(3.) i <= l->len.

(1.) is satisfied upon initialisation of the loop by the principle ex
falso quodlibet.
(2.) similarly holds from the postconditions of fsm_create.
(3.) is satisfied by default, because fsmset_defined(l) implies l->len >= 0.

Now assume that P holds and consider the effect of the loop body (which can only
occur under the added assumption that i != l->len). The conditional assignment
to s->accepting assures us that
	!l->arr[i]->accepting || s->accepting,
which can be re-written as
	!l->arr[(i+1)-1]->accepting || s->accepting.	(B)
Now fsm_defined(l->arr[i]) follows from the precondition fsmset_defined(l), thus
evidently the output e of the edge_create will satisfy the postconditions
	- edge_defined(e)
	- e->dest == l->arr[i] && e->c == '\0'.
But then, given that fsm_defined(s) by assumption of (P.2), we clearly have the
preconditions of fsm_addedge, leading to the postconditions
	- fsm_edgeto(s, l->arr[i], '\0')
	- fsm_defined(s),
which can be re-written as
	- fsm_edgeto(s, l->arr[(i+1)-1], '\0')
	- fsm_defined(s).
This, together with (B), is the precondition that the following will
hold upon execution of the incremental assignment i++:
	(1.)
	fsm_edgeto(s, t, '\0') && (!t->accepting || s->accepting)
		for t := l->arr[j] for j := i-1

	(2.) fsm_defined(s);

which, when taken together with the assumption of P for the previous i, that now
applies on the interval 0 <= j < i-1, clearly gives us (P.1) and (P.2).
Finally, since prior to the assignment we had i != l->len and i <= l->len, then
we had i+1 <= l->len, so the assignment preserves (P.3) also.
This shows the invariance of P. 

To see that termination is guaranteed, consider the function
	t(i) := l->len - i.
Assuming P and i != l->len we get t(i) > 0, because (P.3) and i != l->len imply
that i < l->len.
Consider a fixed value t0 such that
	t(i) <= t0 + 1.	(*)
By the assignment axiom we will have
	t(i) <= t0
after the (assignment) statement i++ if
	t(i+1) <= t0
holds prior. But from (*) we have
	l->len - i <= t0 + 1
which is equivalent to
	l->len - (i+1) <= t0,
or t(i+1) <= t0.
Thus the function t is strictly decreasing with each iteration, and since t == 0
implies l->len == i, the termination condition of the loop, termination is
guaranteed.

When the loop terminates, we have P and i == l->len.
Since our return value is s, note that (P.2) implies our first postcondition
fsm_defined(ret).
For the second postcondition, note that (P.1) has the same predicate (the first
line of both), so the only question is whether the range
	t := l->arr[j] for j in {j := 0; j < l->len; j++}	(1)
is equivalent to
	all struct fsm *t in l,
or more explicitly,
	all π(struct fsm *t)(t in l) l.	(2)
If l->len == 0, then both ranges are Ø and therefore equal.
Now assume that l->len > 0. By definition, (2) is 
	l->arr[i] for i in all (l->arr[i] in l) {i := 0; i < l->len; i++} (3)
But (l->arr[i] in l) is expanded as
	fsmset_nonempty(l) && (all π(struct fsm *t)(l->arr[i] == t) l) != Ø.
Since by assumption we have fsm_defined(l) and l->len > 0, the first term here
is inconsequential. The range in the left-hand of the second term is
	l->arr[k] for k in all (l->arr[i] == l->arr[k]) {k := 0; k < l->len; k++}
At this stage we are forced to appeal to the primitives of our invented
language, in which the "all" operator on a set of integers works in the ordinary
way, and clearly the above set cannot be Ø because in the case k = i the
predicate (l->arr[i] == l->arr[k]) is true. Thus (3) is
	l->arr[i] for i in all true {i := 0; i < l->len; i++},
which (by another appeal to the nature of sets in our language becomes)
	l->arr[i] for i in {i := 0; i < l->len; i++},
which is equivalent to the range (1) — except for the name t.
*/
```


## fsm
```C
struct fsm *
fsm_create(bool accept);

void
fsm_addedge(struct fsm *s, struct edge *e)
{
	s->edges = (struct edge **)
		realloc(s->edges, sizeof(struct edge) * (++s->nedges));
	s->edges[s->nedges-1] = e;
	if (e->c == '\0') {
		s->accepting |= e->dest->accepting;
	}
}
/*
For an integer n, define the predicate
	P(n):
	fsm_edgeto(s, e->dest, e->c) for e := s->edges[i]
		for i in {i := 0; i < n; i++}.
From the precondition fsm_defined(s), we get P(s->nedges) holding initially,
which can be re-written as P((s->nedges+1)-1). Thus the assignment ++s->nedges
implies P(s->nedges-1).

Now P(s->nedges-1) implies that s->edges is pointing to a location in memory
with a size that is at least
	sizeof(struct edge) * (s->nedges-1).
From the C manpages realloc working implies that, after the assignment, s->edges
will point to a location in memory with the size given by
	sizeof(struct edge) * s->nedges,	(S)
and that P(s->nedges-1) will be preserved, because the values of s->edges will
not change for i < s->nedges-1. This latter invariance follows from the
reducibility of fsm_edgeto to fsm_base_defined and edge_points:
fsm_base_defined(s) will not be affected by realloc; edge_points will be
preserved on {i := 0; i < (s->nedges-1); i++}, because the value of s->edge[i]
is preserved by realloc.

Since s->edges now has the size S, the assignment
	s->edges[s->nedges-1] = e
is possible, and it implies 
	E1:
	edge_points(s->edges[i], e->dest, e->c)
		for i = s->nedges-1,
because of our assumption edge_points(e, e->dest, e->c) and
the axiom of assignment. The conditional block guarantees
	E2:
	edge_matchacc(s->edges[i], s->accepting)
		for i = s->nedges-1:
if e->c != '\0' or !e->dest->accepting then E2 holds by default; if 
	e->c == '\0' && e->dest->accepting
then the conditional assignment
	s->accepting |= e->dest->accepting
will set s->accepting to true, upholding E2.
Since i = s->nedges-1 is in the range
	{i := 0; i < s->nedges; i++},
E1 and E2, together with P(s->nedges-1) and the fact that
fsm_base_defined(s) still holds, give us
	fsm_edgeto(s, e->dest, e->c),
which is our second postcondition.

Taken together with P(s->nedges-1), the above implies P(s->nedges), because
	fsm_edgeto(s, e->dest, e->c) for e := s->edges[i],
when i = s->nedges-1. But P(s->nedges) is nothing but our first postcondition.
*/

/* circuitbreaker: tracker to prevent ε-loops */
struct circuitbreaker {
	struct fsm *s;
	struct circuitbreaker *next;
};

int
circuitbreaker_len(struct circuitbreaker *tr)
{
	int n = 0;
	for (struct circuitbreaker *next = tr; tr != NULL; tr = tr->next) {
		n++;
	}
	return n;
}


struct circuitbreaker*
circuitbreaker_create(struct fsm *s)
{
	struct circuitbreaker *tr = (struct circuitbreaker *)
		malloc(sizeof(struct circuitbreaker));
	tr->s = s;
	tr->next = NULL;
	return tr;
}

struct circuitbreaker*
circuitbreaker_copy(struct circuitbreaker *tr)
{
	struct circuitbreaker *new = circuitbreaker_create(tr->s);
	if (tr->next != NULL) {
		new->next = circuitbreaker_copy(tr->next);
	}
	return new;
}


void
circuitbreaker_destroy(struct circuitbreaker *tr)
{
	assert(tr != NULL);
	if (tr->next != NULL) {
		circuitbreaker_destroy(tr->next);
	}
	free(tr);
}

bool
circuitbreaker_append(struct circuitbreaker *tr, struct fsm *s)
{
	for (; tr->s != s; tr = tr->next) {
		if (tr->next == NULL) {
			tr->next = circuitbreaker_create(s);
			return true;
		}
	}
	return false;
}

static struct fsmset *
fsm_move_act(struct fsm *s, char c, struct circuitbreaker *tr)
{
	struct fsmset *l = fsmset_create();
	for (int i = 0; i < s->nedges; i++) {
		struct edge *e = s->edges[i];
		if (e->c == c) {
			fsmset_add(l, e->dest);
		} else if (e->c == '\0') {
			struct circuitbreaker *trnew = circuitbreaker_copy(tr);
			if (circuitbreaker_append(tr, e->dest)) {
				fsmset_addrange(l, 
					fsm_move_act(e->dest, c, trnew));
			}
			circuitbreaker_destroy(trnew);
		}
	}
	return l;
}

static struct fsmset *
fsm_move(struct fsm *s, char c)
{
	struct circuitbreaker *tr = circuitbreaker_create(s);
	struct fsmset fsm_move_act(s, c, tr);
	circuitbreaker_destroy(tr);
	return l;
}

struct fsm *
fsm_sim(struct fsm *s, char c)
{
	struct fsmset *l = fsmset_create();
	// ⊢ fsmset_defined(l) && fsmset_empty(l)
	fsmset_add(l, s);
	/*
	⊢ B.0: s in l
	⊢ (s == t || t in pre(l)) for struct fsm *t in l.

	For an arbitrary struct fsm *t in l, either s == t or t in pre(l).
	But t in pre(l) must be false (by definition) because
	fsmset_empty(pre(l)). Thus
	⊢ B.1: s == t for struct fsm *t in l.
	*/
	struct fsmset *cls = fsmset_epsclosure(l);
	/*
	⊢ fsm_reaches(σ, t) for some struct fsm *σ in l
		for struct fsm *t in cls
	⊢ t in cls for struct fsm *t in 
		all π(struct fsm *t)
		(t in l || fsm_reaches(σ, t) for some struct *σ in l) l

	Given an arbitrary struct fsm *t in cls, we have 
		fsm_reaches(σ, t) for some struct fsm *σ in l.
	But then σ == t from B.1, so fsm_reaches(s, t).
	|- C.0: fsm_reaches(s, t) for struct fsm *t in cls.

	Let t := e->dest for some struct edge *e in s. If fsm_edgeson(s, t, c),
	then either fsm_edgeto(s, t, c), or 
		fsm_defined(s) && fsm_edgeto(s, tt, '\0') && fsm_edgeson(tt, t, c)
		for tt := ee->dest
		for some struct edge *e from s.
	In the former case, since fsmset_epsclosure preserves fsmset_inv(l), we
	know that s in l still holds from B.0. Thus 
		fsm_edgeson(r, t, c) for r := s.
	In the latter case, by definition we have
		fsm_reaches(σ, tt) for σ := s,
	which implies tt is in cls and
		fsm_edgeson(r, t, c) for r := tt.
	⊢ C.1:
	fsm_edgeson(r, t, c) for some struct fsm *r in cls
		|| !fsm_edgeson(s, t, c)
		for t := e->dest for struct edge *e from s;

	Finally, as a consequence of B.0 and the second postcondition of
	fsmset_epsclosure above,
	⊢ C.2: s in cls.
	*/
	struct fsmset *m = fsmset_move(cls, c);
	/*
	Since fsmset_defined(cls) holds as a consequence of C.2, and isletter(c)
	implies the second precondition of fsmset_move, we will have
	⊢ fsmset_defined(m)
	⊢ fsm_edgeson(σ, t, c) for some struct fsm *σ in cls
		for struct fsm *t in m
	⊢ !fsm_edgeson(σ, t, c) || fsm_reaches(r, t) for some struct fsm *r in m
		for t := e->dest for struct edge *e from σ
		for struct fsm *σ in cls.

	For an arbitrary struct fsm *t in m, we have a struct fsm *σ in cls
	such that
		fsm_edgeson(σ, t, c).
	But C.0 tells us that 
		fsm_reaches(s, σ).
	Thus fsm_edgetransitivity(s, σ, t, c) implies
	⊢ D.0: fsm_edgeson(s, t, c) for struct fsm *t in m.

	Next, note the invariance of fsm_inv(cls) implies that C.1 holds from
	above. Let t := ee->dest for some struct edge *ee in s. Then if 
		fsm_edgeson(s, t, c)
	we must have fsm_edgeson(r, t, c) for some struct *r in cls.
	But then we must have
		fsm_reaches(r, t) for some struct fsm *r in m
	because σ in cls for σ := s by C.2. We therefore have
	⊢ D.1:
	fsm_reaches(r, t) for some struct *r in m || !fsm_edgeson(s, t, c)
		for t := e->dest for all π(struct edge *e) (e from s) s.
	*/
	return fsmset_tofsm(m);
}
/*
Because fsmset_defined(m), we will have
⊢ fsm_defined(ret)
⊢ fsm_reaches(ret, t) for struct fsm *t in m
⊢ e->c == '\0' && e->dest in m for struct edge *e from ret.

Let t := e->dest for some struct edge *e from s. Then D.1 tells us that either
fsm_reaches(r, t) for some struct *r in m, or !fsm_edgeson(s, t, c). In the
former case, we will know from the above postconditions that
	fsm_reaches(ret, r) && fsm_reaches(r, t).
If r == t, then we have fsm_reaches(ret, t). Otherwise, we have
	fsm_reaches(ret, r) && fsm_edgeson(r, t, '\0'),
which allows us to apply fsm_edgetransitivity(ret, r, t, '\0') to obtain
	fsm_edgeson(ret, t, '\0')
which implies fsm_reaches(ret, r). Thus
⊢ fsm_reaches(r, t) || !fsm_edgeson(s, t, c)
	for t := e->dest for struct edge *e from s.

Now let t := e->dest for some struct edge *e from ret. The final postcondition
above tells us that (t in m), which from D.0 (because of the invariance
fsmset_inv(m)) fsm_edgeson(s, t, c). Thus
⊢ fsm_edgeson(s, t, c) for t := e->dest for struct edge *e from ret.
*/

```
