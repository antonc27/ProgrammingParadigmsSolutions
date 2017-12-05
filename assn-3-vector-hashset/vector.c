#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  assert(elemSize > 0);
  assert(initialAllocation >= 0);
  v->elemSize = elemSize;
  v->logLength = 0;
  v->allocStep = (initialAllocation == 0) ? 1 : initialAllocation;
  v->allocLength = v->allocStep;
  v->freeFunction = freeFn;
  v->elems = malloc(v->allocLength * v->elemSize);
  assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
  if (v->freeFunction != NULL) {
    for (int i = 0; i < VectorLength(v); i++) {
      v->freeFunction(VectorNth(v, i));
    }
  }
  free(v->elems);
}

int VectorLength(const vector *v)
{
  return v->logLength;
}

void *VectorNth(const vector *v, int position)
{
  assert(position >= 0 && position < VectorLength(v));
  return (char *)v + v->elemSize * position;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position <= VectorLength(v));
  VectorReallocIfNeeded(v);
  int n = VectorLength(v);
  int delta = n - position;
  void *src = (char *)v + v->elemSize * position;
  if (delta > 0) {
    void *dest = (char *)v + v->elemSize * (position + 1);
    memmove(dest, src, v->elemSize * delta);
  }
  memcpy(src, elemAddr, v->elemSize);
  v->logLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
  VectorInsert(v, elemAddr, VectorLength(v));
}

void VectorDelete(vector *v, int position)
{}

void VectorSort(vector *v, VectorCompareFunction compare)
{}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ return -1; } 

void VectorReallocIfNeeded(vector *v)
{
  int n = VectorLength(v);
  if (n + 1 > v->allocLength) {
    v->allocLength += v->allocStep;
    v->elems = realloc(v->elems, v->elemSize * v->allocLength);
    assert(v->elems != NULL);
  }
}
