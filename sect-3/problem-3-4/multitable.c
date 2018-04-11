#include "multitable.h"
#include <stdlib.h>
#include <string.h>

void MultiTableNew(multitable *mt, int keySizeInBytes, int valueSizeInBytes, int numBuckets,
                   MultiTableHashFunction hash,
                   MultiTableCompareFunction compare) {
    mt->keySize = keySizeInBytes;
    mt->valueSize = valueSizeInBytes;
    HashSetNew(&mt->mappings, keySizeInBytes + sizeof(vector), numBuckets, hash, compare, NULL);
}

void MultiTableEnter(multitable *mt, const void *keyAddr, const void *valueAddr) {
    void *existing = HashSetLookup(&mt->mappings, keyAddr);
    if (existing == NULL) {
        void *newKeyValue = malloc(mt->keySize + sizeof(vector));
        memcpy(newKeyValue, keyAddr, mt->keySize);
        vector *value = (vector *)((char *)newKeyValue + mt->keySize);
        VectorNew(value, mt->valueSize, NULL, 1);
        VectorAppend(value, valueAddr);
        HashSetEnter(&mt->mappings, newKeyValue);
        free(newKeyValue);
    } else {
        vector *value = (vector *)((char *)existing + mt->keySize);
        VectorAppend(value, valueAddr);
    }
}

typedef struct {
    MultiTableMapFunction map;
    void *auxData;
    int keySize;
} helpAuxData;

void hashSetMapFunction(void *elemAddr, void *setAuxData) {
    helpAuxData *had = (helpAuxData *)setAuxData;
    vector *value = (vector *)((char *)elemAddr + had->keySize);
    for (int i = 0; i < VectorLength(value); i++) {
        had->map(elemAddr, VectorNth(value, i), had->auxData);
    }
}

void MultiTableMap(multitable *mt, MultiTableMapFunction map, void *auxData) {
    helpAuxData had = {map, auxData, mt->keySize};
    HashSetMap(&mt->mappings, hashSetMapFunction, &had);
}
