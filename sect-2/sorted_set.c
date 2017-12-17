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
  int currentIndex = *((int *)set->base - 1);
  
  while (currentIndex != -1) {
    void *currNodePtr = SetNode(set, currentIndex);
    int cmpResult = set->cmpFn(currNodePtr, elemPtr);
    if (cmpResult == 0) {
      break;
    } else if (cmpResult > 0) {
      currentIndex = *SetNodeLeftIndex(set, currNodePtr);
    } else {
      currentIndex = *SetNodeRightIndex(set, currNodePtr);
    }
  }

  return (currentIndex == -1) ? NULL : SetNode(set, currentIndex);
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

  int previousIndex = -1;
  int currentIndex = *firstIndexPtr;
  int nextIndex = -1;
  int cmpResult;
  
  while (currentIndex != -1) {
    void *currNodePtr = SetNode(set, currentIndex);
    cmpResult = set->cmpFn(currNodePtr, elemPtr);
    if (cmpResult == 0) {
      return false;
    } else if (cmpResult > 0) {
      nextIndex = *SetNodeLeftIndex(set, currNodePtr);
    } else {
      nextIndex = *SetNodeRightIndex(set, currNodePtr);
    }
    previousIndex = currentIndex;
    currentIndex = nextIndex;
  }
  
  int newIndex = set->logLen;
  SetAssignNewNode(set, elemPtr, newIndex);
  
  void *prevNodePtr = SetNode(set, previousIndex);
  if (cmpResult > 0) {
    *SetNodeLeftIndex(set, prevNodePtr) = newIndex;
  } else {
    *SetNodeRightIndex(set, prevNodePtr) = newIndex;
  }

  return true;
}
