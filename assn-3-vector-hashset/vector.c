#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

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
  return (char *)v->elems + v->elemSize * position;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  void *elem = VectorNth(v, position);
  if (v->freeFunction != NULL) {
    v->freeFunction(elem);
  }
  memcpy(elem, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position <= VectorLength(v));
  VectorReallocIfNeeded(v);
  int n = VectorLength(v);
  int delta = n - position;
  void *src = (char *)v->elems + v->elemSize * position;
  if (delta > 0) {
    void *dest = (char *)v->elems + v->elemSize * (position + 1);
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
{
  void *elem = VectorNth(v, position);
  if (v->freeFunction != NULL) {
    v->freeFunction(elem);
  }
  int n = VectorLength(v);
  int delta = n - position;
  void *src = (char *)v->elems + v->elemSize * (position + 1);
  if (delta > 0) {
    void *dest = (char *)v->elems + v->elemSize * position;
    memmove(dest, src, v->elemSize * delta);
  }
  v->logLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  assert(compare != NULL);
  void *base = VectorNth(v, 0);
  int n = VectorLength(v);
  qsort(base, n, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  assert(mapFn != NULL);
  for (int i = 0; i < VectorLength(v); i++) {
    mapFn(VectorNth(v, i), auxData);
  }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
  assert(key != NULL);
  assert(searchFn != NULL);
  assert(startIndex >= 0 && startIndex <= VectorLength(v));
  const void *base = VectorNth(v, startIndex);
  size_t effectiveLength = VectorLength(v) - startIndex;
  void *found = NULL;
  if (isSorted) {
    found = bsearch(key, base, effectiveLength, v->elemSize, searchFn);
  } else {
    found = lfind(key, base, &effectiveLength, v->elemSize, searchFn);    
  }
  if (found == NULL) {
    return kNotFound;
  }
  int foundIndex = ((char *)found - (char *)v->elems) / v->elemSize;
  assert(foundIndex < VectorLength(v));
  return foundIndex;
} 

void VectorReallocIfNeeded(vector *v)
{
  int n = VectorLength(v);
  if (n + 1 > v->allocLength) {
    v->allocLength += v->allocStep;
    v->elems = realloc(v->elems, v->elemSize * v->allocLength);
    assert(v->elems != NULL);
  }
}
