#include "alloc3.h"
#include "morecore.h"
#include <stddef.h>  /* for NULL */

typedef double ALIGN;   /* Force alignment */

union header { /* Header for free block */
  struct h {
    union header *ptr;		/* Next free block */
    int size;			/* Size of this free space */
  } s;
  ALIGN x;			/* Force alignment of the block */
};

typedef union header HEADER;

static HEADER allocbuf  /* Dummy header */
= {{&allocbuf, 1}};	/* ptr points to the dummy header itself. */
      	      	      	/* size	= 1 means zero body size */

static HEADER *allocp = &allocbuf;	/* Last block allocated */

void *alloc3(int nbytes)		/* Return pointer to nbytes block */
{
  /* FILL HERE */
}

/* afree3() is Exactly same as afree2() */

void afree3(void *ap)		/* Free space pointed to by p */
{
  /* omitted */
}
