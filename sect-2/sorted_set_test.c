#include "sorted_set.h"
#include <stdio.h>
#include <assert.h>

int cmpint(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

int main() {
  //printf("Hi\n");
  //void SetNew(sortedset *set, int elemSize, int (*cmpfn)(const void *, const void *))
  int ints[4] = {2, 7, 3, 6};
  
  sortedset set;
  SetNew(&set, sizeof(int), cmpint);

  for (int i = 0; i < 4; i++) {
    SetAdd(&set, ints + i);
  }

  int notFound = 10;
  assert(SetSearch(&set, &notFound) == NULL);

  int found = 6;
  assert(SetSearch(&set, &found) != NULL);
  
  return 0;
}
