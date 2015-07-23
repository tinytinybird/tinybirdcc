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


/* ----- exported types ----- */
enum { PERM=0, FUNC, STMT };

/* exported types: type.c */
struct symbol {
    char *name;
    int scope;
    Coordinate src;
    Symbol up;
    List uses;          /* not done yet */
    int sclass;

    /* symbol flags */
    unsigned temporary:1;
    unsigned generated:1;
    unsigned defined:1;
    unsigned addressed:1;

    Type type;
    float ref;
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
    Xsymbol x;
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
