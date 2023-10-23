#include "alloc.h"

static char allocbuf[ALLOCSIZE]; /* Memory area for alloc */
static char *allocp = allocbuf;	/* Next free position */

void *alloc(int n)		/* Return pointer to n characters */
{
  if (allocbuf + ALLOCSIZE - allocp >= n) { /* we have enough space */
    void *p = allocp;
    allocp += n;
    return p;
  } else			/* not enough space */
    return 0;
}

void afree(void *p)		/* Free space pointed to by p */
{
  if (p >= (void *)allocbuf && p < (void *)allocbuf + ALLOCSIZE)
    allocp = p;
}
