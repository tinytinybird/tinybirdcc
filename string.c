/*
 * String Operation
 * 
 * - Constant strings used for creating identifiers, constants,
 *   registers and so on. 
 * - Only one copy of the strings is saved.
 * - Use hash table to search.
 * 
 */

/* Exported functions */
extern char *string(char *str);
extern char *stringn(char *str, int len);
extern char *stringd(int n);


/* Data */
static struct string {
    char *str;
    int len;
    struct string *link;
} *buckets[1024];

/* Scatter[256]: a static array of 256 random numbers. */


/* Functions */
char *string(char *str)
{
    char *s;
    for (s = str; *s; s++) ;
    return stringn(str, s - str);
}

char *stringd(int n)
{
    char str[25], *s = str + sizeof(str);
    unsigned m;

    if (n == INT_MIN)
	m = (unsigned)INT_MAX + 1;
    else if (n < 0)
	m = -n;
    else
	m = n;

    do
	*--s = m%10 + '0';
    while ((m /= 10) != 0);
    if (n < 0)
	*--s = '-';
    return stringn(s, str + sizeof(str) - s);
}

char *stringn(char *str, int len)
{
    int i;
    unsigned int h;
    char *end;
    struct string *p;

    /* hash function */
    for (h = 0, i = len, end = str; i > 0; i--)
	h = (h<<1) + scatter[*(unsigned char *)end++];
    h &= NELEMS(buckets) - 1;

    for (p = buckets[h]; p; p = p->link)
	if (len == p->len) {
	    char *s1 = str, *s2 = p->str;
	    do {
		if (s1 == end)
		    return p->str;
	    } while (*s1++ == *s2++);
	}
    {
	static char *next, *strlimit;
	if (next + len + 1 >= strlimit) {
	    int n = len + 4*1024;
	    next = allocate(n, PERM);
	    strlimit = next + n;
	}
	NEW(p, PERM);
	p->len = len;
	for (p->str = next; str < end; )
	    *next++ = *str++;
	*next++ = 0;
	p->link = buckets[h];
	buckets[h] = p;
	return p->str;
    }
}

