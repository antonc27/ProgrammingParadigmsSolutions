#include "sorted_set.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Helpers

void SetReallocRealBase(sortedset *set, void **base) {
  size_t nodeSize = set->elemSize + 2 * sizeof(int);
  *base = realloc(*base, sizeof(int) + set->allocLen * nodeSize);
  assert(*base != NULL);
}

void *SetNode(sortedset *set, int index) {
  size_t nodeSize = set->elemSize + 2 * sizeof(int);
  return (char *)set->base + nodeSize * index;
}

int *SetNodeLeftIndex(sortedset *set, void *node) {
  return (int *)((char *)node + set->elemSize);
}

int *SetNodeRightIndex(sortedset *set, void *node) {
  return SetNodeLeftIndex(set, node) + 1;
}

void SetCopyToNewNode(sortedset *set, const void *elemPtr, int newIndex) {
  void *newNode = SetNode(set, newIndex);
  memcpy(newNode, elemPtr, set->elemSize);
  *SetNodeLeftIndex(set, newNode) = -1;
  *SetNodeRightIndex(set, newNode) = -1;
}

static const int kInitialCapacity = 4;
void SetAssignNewNode(sortedset *set, const void *elemPtr, int newIndex) {
  if (set->logLen == set->allocLen) {
    // realloc
    set->allocLen += kInitialCapacity;
    void *base = (int *)set->base - 1;
    SetReallocRealBase(set, &base);
  }
  SetCopyToNewNode(set, elemPtr, newIndex);
  set->logLen++;
}

int *SetFindNodeIndex(sortedset *set, const void *elemPtr) {
  int *firstIndexPtr  = ((int *)set->base - 1);
  int *currIndexPtr = firstIndexPtr;  
  while (*currIndexPtr != -1) {
    void *currNodePtr = SetNode(set, *currIndexPtr);
    int cmpResult = set->cmpFn(currNodePtr, elemPtr);
    if (cmpResult == 0) {
      break;
    } else if (cmpResult > 0) {
      currIndexPtr = SetNodeLeftIndex(set, currNodePtr);
    } else {
      currIndexPtr = SetNodeRightIndex(set, currNodePtr);
    }
  }
  return currIndexPtr;
}

// Public

void SetNew(sortedset *set, int elemSize, int (*cmpfn)(const void *, const void *)) {
  assert(elemSize > 0);
  assert(cmpfn != NULL);
  set->elemSize = elemSize;
  set->cmpFn = cmpfn;
  
  set->allocLen = kInitialCapacity;
  set->logLen = 0;

  void *base = NULL;
  SetReallocRealBase(set, &base);

  *(int *)base = -1;
  set->base = (int *)base + 1;
}

void *SetSearch(sortedset *set, const void *elemPtr) {
  int *currIndexPtr = SetFindNodeIndex(set, elemPtr);
  return (*currIndexPtr == -1) ? NULL : SetNode(set, *currIndexPtr);
}

bool SetAdd(sortedset *set, const void *elemPtr) {
  int *firstIndexPtr = ((int *)set->base - 1);
  if (*firstIndexPtr == -1) {
    // empty set
    SetAssignNewNode(set, elemPtr, 0);
    *firstIndexPtr = 0;
    return true;
  }
  assert(*firstIndexPtr == 0);

  int *currIndexPtr = SetFindNodeIndex(set, elemPtr);
  if (*currIndexPtr != -1) {
    return false;
  }
  
  int newIndex = set->logLen;
  SetAssignNewNode(set, elemPtr, newIndex);
  
  *currIndexPtr = newIndex;

  return true;
}

void SetPrint(sortedset *set, void (*printfn)(const void *elemPtr)) {
  for (int i = 0; i < set->logLen; i++) {
    void *node = SetNode(set, i);
    printfn(node);
    printf(" %d %d\n", *SetNodeLeftIndex(set, node), *SetNodeRightIndex(set, node));
  }
}
