/*
 * Common Declarations
 *
 * - c.h is included by all modules.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/* ----- exported macros ----- */
#define NULL ((void*)0)
#define NELEMS(a) ((int)(sizeof (a)/sizeof ((a)[0])))
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

/* exported macros: alloc.c */
#define NEW(p,a) ((p)) = allocate(sizeof *(p), (a))
#define NEW0(p,a) memset(NEW((p),(a)), 0, sizeof *(p))

/* exported macros: types.c */
#define isqual(t) ((t)->op >= CONST)
#define unqual(t) (isqual(t) ? (t)->type : (t))

#define isvolatile(t) ((t)->op == VOLATILE \
                       || (t)->op == CONST+VOLATILE)
#define isconst(t) ((t)->op == CONST \
                    || (t)->op == CONST+VOLATILE)
#define isarray(t) (unqual(t)->op == ARRAY)
#define isstruct(t) (unqual(t)->op == STRUCT \
                     || unqual(t)->op == UNION)
#define isunion(t) (unqual(t)->op == UNION) 
#define isfunc(t) (unqual(t)->op == FUNCTION)
#define isptr(t) (unqual(t)->op == POINTER)
#define ischar(t) (unqual(t)->op == CHAR)
#define isint(t) (unqual(t)->op >= CHAR \
        && (unqual(t)->op <= UNSIGNED)
#define isfloat(t) (unqual(t)->op <= DOUBLE)
#define isarith(t) (unqual(t)->op <= UNSIGNED)
#define isunsigned(t) (unqual(t)->op == UNSIGNED)
#define isdouble(t) (unqual(t)->op == DOUBLE)
#define isscalar(t) (unqual(t)->op <= POINTER \
                     || unqual(t)->op == ENUM)
#define isenum(t) (unqual(t)->op == ENUM)

#define fieldsize(p) (p)->bitsize
#define fieldright(p) ((p)->lsb - 1)
#define fieldleft(p) (8*(p)->type->size - \
                      fieldsize(p) - fieldright(p))
#define fieldmask(p) (~(~(unsigned)0<<fieldsize(p)))

#define widen(t) (isint(t) || isenum(t) ? INT : ttob(t))



/* ----- typedefs ----- */
typedef struct node *Node;

/* typedefs: sym.c */
typedef struct symbol *Symbol;
typedef struct coord {
    char *file;
    unsigned x, y;
} Coordinate;

typedef struct table *Table;

typedef union value {
    /* signed */ char sc;
    short ss;
    int i;
    unsigned char uc;
    unsigned short us;
    unsigned int u;
    float f;
    double d;
    void *p;
} Value;

/* typedefs: types.c */
typedef struct type *Type;
typedef struct field *Field;



/* #include "config.h" */

/* interface */

typedef struct metrics {
    unsigned char size, align, outofline;
} Metrics;

typedef struct interface {
    /* metrics */
    Metrics charmetric;
    Metrics shortmetric;
    Metrics intmetric;
    Metrics floatmetric;
    Metrics doublemetric;
    Metrics ptrmetric;
    Metrics structmetric;

    /* interface flags */
    unsigned little_endian:1;
    unsigned mulops_calls:1;
    unsigned wants_callb:1;
    unsigned wants_argb:1;
    unsigned left_to_right:1;
    unsigned wants_dag:1;

    /* interface functions */
    void (*progbeg) (int argc, char *argv[]);
    void (*progend) (void);
    void (*defsymbol) (Symbol);
    void (*export) (Symbol);
    void (*import) (Symbol);
    void (*global) (Symbol);
    void (*local) (Symbol);
    void (*address) (Symbol p, Symbol q, int n);
    void (*segment) (int);
    void (*defaddress) (Symbol);
    void (*defconst) (int ty, Value v);
    void (*defstring) (int n, char *s);
    void (*space) (int);
    void (*function) (Symbol, Symbol[], Symbol[], int);
    void (*emit) (Node);
    void (*gen) (Node);
    void (*blockbeg) (Env *);
    void (*blockend) (Env *);
    
    /* functions for debug info output */
    void (*stabblock) (int, int, Symbol*);
    void (*stabend) (Coordinate *, Symbol, Coordinate **, Symbol *, Symbol *);
    void (*stabfend) (Symbol, int);
    void (*stabinit) (char *, int, char *[]);
    void (*stabline) (Coordinate *);
    void (*stabsym) (Symbol);
    void (*stabtype) (Symbol);

    Xinterface x;
} Interface;


/* ----- exported types ----- */
enum { PERM=0, FUNC, STMT };

struct node {
    short op;
    short count;
    Symbol syms[3];
    Node kids[2];
    Node link;
    Xnode x;
};

enum {
    F=FLOAT,
    D=DOUBLE,
    C=CHAR,
    S=SHORT,
    I=INT,
    U=UNSIGNED,
    P=POINTER,
    V=VOID,
    B=STRUCT
};

enum {
    CNST=1<<4,
    CNSTC=CNST+C,
    CNSTD=CNST+D,
    CNSTF=CNST+F,
    CNSTI=CNST+I,
    CNSTP=CNST+P,
    CNSTS=CNST+S,
    CNSTU=CNST+U,

    ARG=2<<4,
    ARGB=ARG+B,
    ARGD=ARG+D,
    ARGF=ARG+F,
    ARGI=ARG+I,
    ARGP=ARG+P
};

enum { CODE=1, BSS, DATA, LIT };

/* exported types: type.c */
struct symbol {
    char *name;
    int scope;          /* GLOBAL, PARAM, LOCAL+k */
    Coordinate src;
    Symbol up;
    List uses;          /* not done yet */
    int sclass;         /* STATIC, AUTO, EXTERN, REGISTER */

    /* symbol flags */
    unsigned temporary:1;
    unsigned generated:1;
    unsigned defined:1;
    unsigned addressed:1;

    Type type;
    float ref;       /* approximates the number of times */
                     /* the variable or label is referenced */

    union {
	/* labels */
	struct {
	    int label;
	    Symbol equatedto;
	} l;
	/* struct types */
        struct {
            unsigned cfields:1;
            unsigned vfields:1;
            Field flist;
        } s;
	/* enum constants */
        int value;
	/* enum types */
        Symbol *idlist;
	/* constants */
	struct {
	    Value v;
	    Symbol loc;
	} c;
	/* function symbols */
	/* globals */
	/* temporaries */
    } u;
    Xsymbol x;         /* target-specific data only for back end */
    /* debugger extension */
};

enum { CONSTANTS=1, LABELS, GLOBAL, PARAM, LOCAL };

/* exported types: types.c */
struct type {
    int op;
    Type type;  /* operand */
    int align;
    int size;
    union {
        Symbol sym;
        /* function types */
        struct {
            unsigned oldstyle:1;
            Type *proto;
        } f;
    } u;
    Xtype x;
};

struct field {
    char *name;
    Type type;
    int offset;
    short bitsize;
    short lsb;
    Field link;
};




/* ----- exported data ----- */
/* exported data: sym.c */
extern Table constants;
extern Table externals;
extern Table globals;
extern Table identifiers;
extern Table labels;
extern Table types;

extern int level;

/* exported data: types.c */
extern Type chartype;
extern Type doubletype;
extern Type floattype;
extern Type inttype;
extern Type longdouble;
extern Type longtype;
extern Type shorttype;
extern Type signedchar;
extern Type unsignedchar;
extern Type unsignedlong;
extern Type unsignedshort;
extern Type unsignedtype;
extern Type voidptype;
extern Type voidtype;



/* ----- exported functions ----- */
/* exported functions: alloc.c */
extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);

/* exported functions: string.c */
extern char *string(char *str);
extern char *stringn(char *str, int len);
extern char *stringd(int n);

/* exported functions: dag.c */
extern void emitcode (void);
extern void gencode (Symbol[], Symbol[]);
