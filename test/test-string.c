/* Exported macros */
#define NEW(p,a) ((p)) = allocate(sizeof *(p), (a))
#define NEW0(p,a) memset(NEW((p),(a)), 0, sizeof *(p))

/* Exported functions */
extern void *allocate(unsigned long n, unsigned a);
extern void deallocate(unsigned a);
extern void *newarray(unsigned long m, unsigned long n, unsigned a);

#include "string.c"

int main(int argc, char *argv[])
{
    char *p = string("abcde");
    char *q = stringd(123);
    char *r = stringn("abcde", 2);
    char *s = string("123");

    printf("%s\n", p);
    printf("%s\n", q);
    printf("%s\n", r);
    printf("q = %x, s = %x\n", q, s);
    
    return 0;
}
