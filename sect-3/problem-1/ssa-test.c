#include "ssa.h"

#include <string.h>
#include <stdio.h>

static void CountEmptyPrintNonEmpty(int index, const char *str, void *auxData) {
    if (strcmp(str, "") != 0) {
        printf("Oooo! Nonempty string at index %d: \"%s\"\n", index, str);
    } else {
       (*(int *)auxData)++;
    }
}

int main(int argc, char **argv) {
    sparsestringarray ssa;
    SSANew(&ssa, 70000, 35);

    SSAInsert(&ssa, 33001, "need");
    SSAInsert(&ssa, 58291, "more");
    SSAInsert(&ssa, 33000, "Eye");
    SSAInsert(&ssa, 33000, "I");
    SSAInsert(&ssa, 67899, "cowbell");

    int numEmptyStrings = 0;
    SSAMap(&ssa, CountEmptyPrintNonEmpty, &numEmptyStrings);
    printf("%d of the strings were empty strings.\n", numEmptyStrings);

    SSADispose(&ssa);
    return 0;
}