#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
  assert(elemSize > 0);
  assert(numBuckets > 0);
  assert(hashfn != NULL);
  assert(comparefn != NULL);
  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->hashFn = hashfn;
  h->compareFn = comparefn;
  h->freeFn = freefn;
  h->buckets = (vector **) malloc(sizeof(vector *) * numBuckets);
  for (int i = 0; i < numBuckets; i++) {
    vector v;
    VectorNew(&v, elemSize, freefn, 1);
    h->buckets[i] = &v;
  }
}

void HashSetDispose(hashset *h)
{
  for (int i = 0; i < h->numBuckets; i++) {
    VectorDispose(h->buckets[i]);
  }
  free(h->buckets);
}

int HashSetCount(const hashset *h)
{
  int count = 0;
  for (int i = 0; i < h->numBuckets; i++) {
    count += VectorLength(h->buckets[i]);
  }
  return count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  void *existing = HashSetLookup(h, elemAddr);
  int bucketIdx = h->hashFn(elemAddr, h->numBuckets);
  vector *bucket = h->buckets[bucketIdx];
  if (existing == NULL) {
    VectorAppend(bucket, elemAddr);
  } else {
    int elemIdx = VectorSearch(bucket, elemAddr, h->compareFn, 0, false);
    VectorReplace(bucket, elemAddr, elemIdx);
  }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);
  int bucketIdx = h->hashFn(elemAddr, h->numBuckets);
  assert(bucketIdx >= 0 && bucketIdx < h->numBuckets);
  vector *bucket = h->buckets[bucketIdx];
  int elemIdx = VectorSearch(bucket, elemAddr, h->compareFn, 0, false);
  return (elemIdx == -1) ? NULL : VectorNth(bucket, elemIdx);
}
