/*
 * Symbol Table Module
 * 
 */

#include "c.h"

/* macros */
#define equalp(x) v.x == p->sym.u.c.v.x


/* types */
struct table {
    int level;
    Table previous;
    struct entry {
	struct symbol sym;
	struct entry *link;
    } *buckets[256];
    Symbol all;
};

#define HASHSIZE NELEMS(((Table)0)->buckets) /* 0-> ? */


/* data */
static struct table
    cns = { CONSTANTS },
    ext = { GLOBAL },
    ids = { GLOBAL },
    tys = { GLOBAL };
Table constants = &cns;
Table externals = &ext;
Table identifiers = &ids;
Table globals = &ids;
Table types = &tys;
Table labels;

Coordinate src;

int level = GLOBAL;


/* functions */
Table table(Table tp, int level)
{
    Table new;

    NEW0(new, FUNC);
    new->previous = tp;
    new->level = level;
    if (tp)
	new->all = tp->all;
    return new;
}

void foreach(Table tp, int lev, void (*apply) (Symbol, void *), void *cl)
{
    while (tp && tp->level > lev)
	tp = tp->previous;
    if (tp && tp->level == lev) {
	Symbol p;
	Coordinate sav;
	sav = src;
	for (p = tp->all; p && p->scope == level; p = p->up) {
	    src = p->src;
	    (*apply)(p, cl);
	}
	src = sav;
    }
}

void enterscope()
{
    ++level;
}

void exitscope()
{
    rmtypes(level);
    if (types->level == level)
	types = types->previous;
    if (identifiers->level == level) {
	/* warn if more than 127 identifiers */
	identifiers = identifiers->previous;
    }
    --level;
}

Symbol install(char *name, Table *tpp, int level, int arena)
{
    Table tp = *tpp;
    struct entry *p;
    unsigned h = (unsigned)name&(HASHSIZE-1);

    if (level > 0 && tp->level < level) /* allocate a new table for  */
	tp = *tpp = table(tp, level);   /* the scope */
    NEW0(p, arena);
    p->sym.name = name;
    p->sym.scope = level;
    p->sym.up = tp->all;
    tp->all = &p->sym;
    p->link = tp->buckets[h];
    tp->buckets[h] = p;
    return &p->sym;
}

Symbol lookup(char *name, Table tp, struct entry *p)
{
    unsigned h = (unsigned)name&(HASHSIZE-1);

    do
	for (p = tp->buckets[h]; p; p = p->link)
	    if (name == p->sym.name)
		return &p->sym;
    while ((tp = tp->previous) != NULL);

    return NULL;
}

int genlabel(int n)
{
    static int label = 1;

    label += n;
    return label - n;
}

Symbol findlabel(int lab)
{
    struct entry *p;
    unsigned h = lab&(HASHSIZE-1);

    for (p = labels->buckets[h]; p; p = p->link)
	if (lab == p->sym.u.l.label)
	    return &p->sym;
    NEW0(p, FUNC);
    p->sym.name = stringd(lab);
    p->sym.scope = LABELS;
    p->sym.up = labels->all;
    labels->all = &p->sym;
    p->link = labels->buckets[h];
    labels->buckets[h] = p;
    p->sym.generated = 1;
    p->sym.u.l.label = lab;
    (*IR->defsymbol)(&p->sym);

    return &p->sym;
}

Symbol constant(Type ty, Value v)
{
    struct entry *p;
    unsigned h = v.u&(HASHSIZE-1); /* treat v's value as u (unsigned) */

    ty = unqual(ty);
    for (p = constants->buckets[h]; p; p = p->link)
	if (eqtype(ty, p->sym.type, 1))
	    switch (ty->op) {
	    case CHAR:      if (equalp(uc)) return &p->sym; break;
	    case SHORT:     if (equalp(ss)) return &p->sym; break;
	    case INT:       if (equalp(i))  return &p->sym; break;
	    case UNSIGNED:  if (equalp(u))  return &p->sym; break;
	    case FLOAT:     if (equalp(f))  return &p->sym; break;
	    case DOUBLE:    if (equalp(d))  return &p->sym; break;
	    case ARRAY:
	    case FUNCTION:
	    case POINTER:   if (equalp(p))  return &p->sym; break;
	    }
    NEW0(p, PERM);
    p->sym.name = vtoa(ty,v);
    p->sym.scope = CONSTANTS;
    p->sym.type = ty;
    p->sym.sclass = STATIC;
    p->sym.u.c.v = v;
    p->link = constants->buckets[h];
    constants->buckets[h] = p;
    p->sym.up = constants->all;
    constants->all = &p->sym;
    if (ty->u.sym && !ty->u.sym->addressed)
	(*IR->defsymbol)(&p->sym);
    p->sym.defined = 1;
    return &p->sym;
}

Symbol intconst(int n)
{
    Value v;
    v.i = n;
    return constant(inttype, v);
}

Symbol genident(int scls, Type ty, int lev)
{
    Symbol p;

    NEW0(p, lev >= LOCAL ? FUNC : PERM);
    p->name = stringd(genlabel(1));
    p->scope = lev;
    p->sclass = scls;
    p->type = ty;
    p->generated = 1;
    if (lev == GLOBAL)
	(*IR->defsymbol)(p);
    return p;
}

Symbol temporary(int scls, Type ty, int lev)
{
    Symbol p = genident(scls, ty, lev);

    p->temporary = 1;

    return p;
}

Symbol newtemp(int sclass, int tc)
{
    Symbol p = temporary(sclass, btot(tc), LOCAL);

    (*IR->local)(p);
    p->defined = 1;
    
    return p;
}

