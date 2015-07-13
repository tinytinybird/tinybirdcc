#include "alloc.c"

struct strt {
    int a;
    float b;
    int arr[10];
};


int main(int argc, char *argv[])
{
    struct strt *p = NEW(p, PERM);
    p->a = 1;
    p->b = 1.0;
    printf("pointer a in p = %d\n", p->a);
    printf("pointer b in p = %f\n", p->b);
    
    struct strt *q = NEW0(q, PERM);
    printf("pointer a in q = %d\n", q->a);

    return 0;
}
