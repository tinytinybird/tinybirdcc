/* 
 * Arena based Memory Allocation
 * 
 * - blocks are allocated in tree arenas
 * - deallocate one whole arena each time
 *
 */

#include "c.h"


/* Types */
struct block {
    struct block *next;
    char *limit;
    char *avail;
};

union align {
    long l;
    char *p;
    double d;
    int (*f) (void);
};

union header {
    struct block b;
    union align a;
};


/* Data */
static struct block
    first[] = { { NULL }, { NULL }, { NULL } }, /* initiate with only one NULL? */
    *arena[] = { &first[0], &first[1], &first[2] };

static struct block *freeblocks;


/* Functions */
void *allocate(unsigned long n, unsigned a)
{
    struct block *ap;

    ap = arena[a];
    n = roundup(n, sizeof(union align));
    while (ap->avail + n > ap->limit) {
	if ((ap->next = freeblocks) != NULL) {
	    freeblocks = freeblocks->next;
	    ap = ap->next;
	} else {
	    unsigned m = sizeof (union header) + n + 10*1024;
	    ap->next = malloc(m);
	    ap = ap->next;
	    if (ap == NULL) {
		error("insufficient memory\n");
		exit(1);
	    }
	    ap->limit = (char *)ap + m;	  /* limit is set up at the first time */
	}                                 /* when the block is allocated */
	/* set up avail and next for a block, new or from freeblocks */
	ap->avail = (char *)((union header *)ap + 1);
	ap->next = NULL;
	arena[a] = ap;
    }
    ap->avail += n;
    return ap->avail - n;
}

void *newarray(unsigned long m, unsigned long n, unsigned a)
{
    return allocate(m*n, a);
}

void deallocate(unsigned a) {
    arena[a]->next = freeblocks;
    freeblocks = first[a].next;
    first[a].next = NULL;
    arena[a] = &first[a];
}

