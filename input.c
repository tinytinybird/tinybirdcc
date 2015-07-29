/*
 * Input module
 *
 * - reads the input in large chunks into a buffer
 *
 */


/* data */
static int bsize;
static unsigned char buffer[MAXLINE+1 + BUFSIZE+1];

/* functions */
void inputInit()
{
    limit = cp = &buffer[MAXLINE+1];
    bsize = -1;
    lineno = 0;
    file = NULL;
    /* refill buffer */
    fillbuf();
    if (cp >= limit)
        cp = limit;
    nextline();
}

void nextline()
{
    do {
        if (cp >= limit) {
            /* refill buffer */
            fillbuf();
            if (cp >= limit)
                cp = limit;
            if (cp == limit)
                return;
        } else
            lineno++;
        for (line = (char *)cp; *cp == ' ' || *cp == '\t'; cp++)
            ;
    } while (*cp == '\n' && cp == limit);
    if (*cp = '#') {
        resynch();
        nextline();
    }
}

void fillbuf()
{
    if (bsize == 0)
        return;
    if (cp >= limit)
        cp = &buffer[MAXLINE+1];
    else {   /* move the tail portion */
        int n = limit - cp;
        unsigned char *s = &buffer[MAXLINE+1] - n;
        line = (char *)s - ((char *)cp - line);
        while (cp < limit)
            *s++ = *cp++;
        cp = &buffer[MAXLINE+1] - n;
    }
    bsize = read(infd, &buffer[MAXLINE+1], BUFSIZE);
    if (bsize < 0) {
        error("read error\n");
        exit(1);
    }
    limit = &buffer[MAXLINE+1+bsize];
    *limit = '\n';
}
