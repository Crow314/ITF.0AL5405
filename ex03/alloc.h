#ifndef ALLOC_H
#define ALLOC_H

#define ALLOCSIZE (1024 * 1024)

extern void *alloc(int n);
extern void afree(void *p);
#endif
