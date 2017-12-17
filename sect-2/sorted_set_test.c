#include "sorted_set.h"
#include <stdio.h>
#include <assert.h>

int cmpint(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

int main() {
  //printf("Hi\n");
  //void SetNew(sortedset *set, int elemSize, int (*cmpfn)(const void *, const void *))
  int ints[5] = {2, 7, 3, 6, 11};
  
  sortedset set;
  SetNew(&set, sizeof(int), cmpint);

  for (int i = 0; i < 5; i++) {
    bool success = SetAdd(&set, ints + i);
    assert(success);
  }

  int duplicate = 7;
  bool fail = SetAdd(&set, &duplicate);
  assert(!fail);

  int notFound = 10;
  assert(SetSearch(&set, &notFound) == NULL);

  int found = 6;
  assert(SetSearch(&set, &found) != NULL);
  
  return 0;
}
