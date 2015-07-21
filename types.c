/*
 * Type Management
 *
 */

/* exported macros */
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


/* typedefs */
typedef struct type *Type;


/* exported types */
struct type {
    int op;
    Type type;  /* operand */
    int align;
    int size;
    union {
        Symbol sym;
        /* function types */
    } u;
    Xtype x;
};


/* exported data */
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


/* data */
static struct entry {
    struct type type;
    struct entry *link;
} *typetable[128];

static int maxlevel;

/* the symbol table entry for all pointer types */
static Symbol pointersym;


/* functions */
static Type type(int op, int size, int align, Type ty, void *sym)
{
    unsigned h = (op^((unsigned)ty>>3))&(NELEMS(typetable)-1);
    struct entry *tn;

    if (op != FUNCTION && (op != ARRAY || size > 0)) {
        for (tn = typetable[h]; tn; tn = tn->link)
            if (tn->type.op == op     && tn->type.type  == ty &&
                tn->type.size == size && tn->type.align == align &&
                tn->type.u.sym == sym)
                return &tn->type;
    }
    
    NEW(tn, PERM);
    tn->type.op = op;
    tn->type.type = ty;
    tn->type.size = size;
    tn->type.align = align;
    tn->type.u.sym = sym;
    memset(&tn->type.x, 0, sizeof(tn->type.x));
    tn->link = typetable[h];
    typetable[h] = tn;
    return &tn->type;
}

void typeInit()
{
# define xx(v,name,op,metrics) { \
        Symbol p = install(string(name), &types, GLOBAL, PERM);\
        v = type(op, 0, IR->metrics.size, IR->metrics.align, p);\
        p->type = v; p->addressed = IR->metrics.outofline; }

    xx(chartype, "char", CHAR, charmetric);
    xx(doubletype, "double", DOUBLE, doublemetric);
    xx(floattype, "float", FLOAT, floatmetric);
    xx(inttype, "int", INT, intmetric);
    xx(longdouble, "long double", DOUBLE, doublemetric);
    xx(longtype, "long int", INT, intmetric);
    xx(shorttype, "short", SHORT, shortmetric);
    xx(signedchar, "signed char", CHAR, charmetric);
    xx(unsignedchar, "unsigned char", CHAR, charmetric);
    xx(unsignedlong, "unsigned long", UNSIGNED, intmetric);
    xx(unsignedshort, "unsigned short", SHORT, shortmetric);
    xx(unsignedtype, "unsigned int", UNSIGNED, intmetric);
#undef xx

    {   /* The type void has no metrics */
        Symbol p;
        p = install(string("void"), &types, GLOBAL, PERM);
        voidtype = type(VOID, NULL, 0, 0, p);
        p->type = voidtype;
    }

    /* initiate symbol table for pointers */
    pointersym = install(string("T*"), &types, GLOBAL, PERM);
    pointersym->addressed = IR->ptrmetric.outofline;
    voidptype = ptr(voidptype);
}

void rmtypes(int lev)
{
    if (maxlevel >= lev) {
        int i;
        maxlevel = 0;       /* recompute maxlevel */
        for (i = 0; i < NELEMS(typetable); i++) {
            struct entry *tn, **tq = &typetable[i];
            while ((tn = *tq) != NULL)
                if (tn->type.op == FUNCTION)
                    tq = &tn->link;
                else if (tn->type.u.sym && tn->type.u.sym->scope >= lev)
                    *tq = tn->link;
                else {
                    /* recompute maxlevel */
                    if (tn->type.us.sym && tn->type.u.sym->scope > maxlevel)
                        maxlevel = tn->type.u.sym->scope;
                    tq = &tn->link;
                }
        }
    }
}

Type ptr(Type ty)
{
    return type(POINTER, ty, IR->ptrmetric.size,
                IR->ptrmetric.align, pointersym);
}

Type deref(Type ty)
{
    if (isptr(ty))
        ty = ty->type;
    else
        error("type error: %s\n", "pointer expercted");
    return isenum(ty) ? unqual(ty)->type : ty;
}

Type array(Type ty, int n, int a)
{
    if (isfunc(ty)) {
        error("illegal type 'array of %t'\n", ty);
        return array(inttype, n, 0);
    }
    if (level > GLOBAL && isarray(ty) && ty->size == 0)
        error("missing array size\n");
    if (ty->size == 0) {
        if (unqual(ty) == voidptype)
            error("illegal type 'array of %t'\n", ty);
        else if (Aflag >= 2)
            warning("declaring type 'array of %t' is undefined\n", ty);
    } else if (n > INT_MAX/ty->size) {
        error("size of 'array of %t' exceeds %d bytes\n", ty, INT_MAX);
        n = 1;
    }
    return type(ARRAY, ty, n*ty->size, a ? a : ty->align, NULL);
}

Type atop(Type ty)
{
    if (isarray(ty))
        return ptr(ty->type);
    error("type error: %s\n", "array expected");
    return ptr(ty);
}

Type qual(int op, Type ty)
{
    if (isarray(ty))
        ty = type(ARRAY, qual(op, ty->type), ty->size, ty->align, NULL);
    else if (isfunc(ty))
        warning("qualified function type ignored\n");
    else if (isconst(ty) && op == CONST
          || isvolatile(ty) && op == VOLATILE)
        error("illegal type '%k %t'\n", op, ty);
    else {
        if (isqual(ty)) {
            op += ty->op;
            ty = ty->type;
        }
        ty = type(op, ty, ty->size, ty->align, NULL);
    }
    return ty;
}
