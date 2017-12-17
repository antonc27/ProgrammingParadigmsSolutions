#include "sorted_set.h"
#include <stdio.h>
#include <assert.h>

int cmpint(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

void TestInts() {
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
}

int DoubleCmp(const void *v1, const void *v2)
{
  const double *dp1 = v1;
  const double *dp2 = v2;
  double diff = *dp1 - *dp2;

  if (diff > 0.0) return 1;
  if (diff < 0.0) return -1;
  return 0;
}

void PrintDoubleElem(const void *elemPtr) {
  const double *d = elemPtr;
  printf("%.3f", *d);
} 

void TestDoubles() {
  double doubles[4] = {5.245, 8.214, 6.001, 2.81828};

  sortedset set;
  SetNew(&set, sizeof(double), DoubleCmp);

  for (int i = 0; i < 4; i++) {
    bool success = SetAdd(&set, doubles + i);
    assert(success);
  }

  SetPrint(&set, PrintDoubleElem);
}

int main() {
  TestInts();
  TestDoubles();
  return 0;
}
