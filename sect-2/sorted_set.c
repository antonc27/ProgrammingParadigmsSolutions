#include "sorted_set.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static const int kInitialCapacity = 4;
void SetNew(sortedset *set, int elemSize, int (*cmpfn)(const void *, const void *)) {
  assert(elemSize > 0);
  assert(cmpfn != NULL);
  set->index = 0;
  set->elemSize = elemSize;
  set->cmpFn = cmpfn;

  size_t nodeSize = elemSize + 2 * sizeof(int);
  set->base = malloc(kInitialCapacity * nodeSize);
  assert(set->base != NULL);
  
  set->allocLen = malloc(sizeof(int));
  *set->allocLen = kInitialCapacity;

  set->logLen = malloc(sizeof(int));
  *set->logLen = 0;
}

void *SetSearch(sortedset *set, const void *elemPtr) {
  return 0;
}

bool SetAdd(sortedset *set, const void *elemPtr) {
  return false;
}
