#include "sorted_set.h"
#include <stdio.h>
#include <assert.h>

static const int kInitialCapacity = 4;
void SetNew(sortedset *set, int elemSize, int (*cmpfn)(const void *, const void *)) {
  assert(elemSize > 0);
  assert(cmpfn != NULL);
  set->index = 0;
  set->elemSize = elemSize;
  set->cmpFn = cmpfn;
}

void *SetSearch(sortedset *set, const void *elemPtr) {
  return 0;
}

bool SetAdd(sortedset *set, const void *elemPtr) {
  return false;
}
