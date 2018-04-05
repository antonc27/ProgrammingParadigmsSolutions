#include "ssa.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void StringFreeFunction(void *elemAddr) {
    free(*(char **)elemAddr);
}

void SSANew(sparsestringarray *ssa, int arrayLength, int groupSize) {
    assert(arrayLength > groupSize);
    assert(arrayLength % groupSize == 0);
    int numGroups = arrayLength / groupSize;
    ssa->groups = malloc(numGroups * sizeof(group));
    ssa->arrayLength = arrayLength;
    ssa->groupSize = groupSize;
    for (int i = 0; i < numGroups; i++) {
        group g;
        g.bitmap = malloc(groupSize * sizeof(bool));
        for (int j = 0; j < groupSize; j++) {
            g.bitmap[j] = false;
        }
        VectorNew(&g.strings, sizeof(char **), StringFreeFunction, 0);
        ssa->groups[i] = g;
//        memcpy(&ssa->groups[i], &g, sizeof(group));
    }
}

void SSADispose(sparsestringarray *ssa) {
    int numGroups = ssa->arrayLength / ssa->groupSize;
    for (int i = 0; i < numGroups; i++) {
        group *g = &ssa->groups[i];
        VectorDispose(&g->strings);
        free(g->bitmap);
    }
    free(ssa->groups);
}

bool SSAInsert(sparsestringarray *ssa, int index, const char *str) {
    int groupIdx = index / ssa->groupSize;
    int stringsIdx = index % ssa->groupSize;
    
    group *g = &ssa->groups[groupIdx];
    int countNonEmpty = 0;
    for (int i = 0; i <= stringsIdx; i++) {
        if (g->bitmap[i]) {
            countNonEmpty++;
        }
    }
    
    char *strToInsert = strdup(str);
    int insertPosition = countNonEmpty;
    bool isFirstTime = !(g->bitmap[stringsIdx]);
    if (isFirstTime) {
        g->bitmap[stringsIdx] = true;
        VectorInsert(&g->strings, &strToInsert, insertPosition);
        return true;
    } else {
        VectorReplace(&g->strings, &strToInsert, insertPosition);
        return false;
    }
}

void SSAMap(sparsestringarray *ssa, SSAMapFunction mapfn, void *auxData) {
    char *empty = "";
    int numGroups = ssa->arrayLength / ssa->groupSize;
    for (int i = 0; i < numGroups; i++) {
        group *g = &ssa->groups[i];
        int nonEmptyCount = 0;
        for (int j = 0; j < ssa->groupSize; j++) {
            if (g->bitmap[j]) {
                char *str = *(char **)VectorNth(&g->strings, nonEmptyCount);
                mapfn(i*ssa->groupSize + j, str, auxData);
                nonEmptyCount++;
            } else {
                mapfn(i*ssa->groupSize + j, empty, auxData);
            }
        }
    }
}
