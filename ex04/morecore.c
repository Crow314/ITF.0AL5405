#include "morecore.h"
#include <sys/mman.h> /* for mmap(), PROT_*, MAP_* */
#include <stddef.h>  /* for NULL */

static int total = 0;

void *morecore(int nbytes, int *realbytes)
{
  void *cp;

  *realbytes = (nbytes + CORE_UNIT - 1) / CORE_UNIT * CORE_UNIT;

  if (total + *realbytes > CORE_LIMIT) {
    /* Attempt to allocate byond limit. */
    return NULL;
  } 

  cp = mmap(NULL, *realbytes, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
  if (cp == (void *)-1) {
    return NULL;
  }
  total += *realbytes;

  return cp;
}

int get_totalcore(void) { return total; }
void set_totalcore(int t) { total = t; }
