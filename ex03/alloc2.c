#include "alloc2.h"

typedef double ALIGN;   /* Force alignment */

union header { /* Header for free block */
  struct h {
    union header *ptr; /* Next free block */
    int size; /* Size of this free space (including header itself) */
  } s;
  ALIGN x; /* Force alignment of the block */
};

typedef union header HEADER;

static HEADER allocbuf[ALLOCSIZE / sizeof(HEADER)]  /* Memory area for alloc */
= { { {allocbuf,  (ALLOCSIZE / sizeof(HEADER))} }};

static HEADER *allocp = allocbuf; /* Last block allocated */

void *alloc2(int nbytes) /* Return pointer to nbytes block */
{
  HEADER *p, *q;
  int nunits = 1 + (nbytes + sizeof(HEADER) - 1) / sizeof(HEADER);

  for (q = allocp, p = q->s.ptr; ; q = p, p = p->s.ptr) {
    if (p->s.size >= nunits) {
      if (p->s.size == nunits) { /* Just */
        if (p == q) { /* We have only one element */
          return 0;
        }
        q->s.ptr = p->s.ptr;
      } else { /* Allocate tail */
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      allocp = q;
      return (void *)(p + 1);
    }
    if (p == allocp) {
      return 0;
    }
  }
}

void afree2(void *ap) /* Free space pointed to by p */
{
  HEADER *p, *q;
  p = (HEADER *)ap - 1;
  for (q = allocp; !(p > q && p < q->s.ptr); q = q->s.ptr) {
    if (q >= q->s.ptr && (p > q || p < q->s.ptr)) {
      break;
    }
  }
  if (p + p->s.size == q->s.ptr) { /* Merge to upward */
    p->s.size += q->s.ptr->s.size;
    p->s.ptr = q->s.ptr->s.ptr;
  } else {
    p->s.ptr = q->s.ptr;
  }
  if (q + q->s.size == p) { /* Merge to downward */
    q->s.size += p->s.size;
    q->s.ptr = p->s.ptr;
  } else {
    q->s.ptr = p;
  }
  allocp = q;
}
