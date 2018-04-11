#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "multitable.h"

static const signed long kHashMultiplier = -1664117991L;
static int StringHash(const char *s, int numBuckets)
{
    int i;
    unsigned long hashcode = 0;
    for (i = 0; i < strlen(s); i++) {
        hashcode = hashcode * kHashMultiplier + tolower(s[i]);
    }
    return hashcode % numBuckets;
}

static int ZipCodeHash(const void *keyAddr, int numBuckets) {
    char *zipcode = (char *)keyAddr;
    return StringHash(zipcode, numBuckets);
}

static int ZipCodeCompare(const void *keyAddr1, const void *keyAddr2) {
    return strcmp((char *)keyAddr1, (char *)keyAddr2);
}

static void InRangePrint(void *keyAddr, void *valueAddr, void *auxData) {
    char *zipcode = (char *)keyAddr;
    char *city = *(char **)valueAddr;
    char *low = *((char **)auxData);
    char *high = *((char **)auxData + 1);
    
    if ((strcmp(zipcode, low) >= 0) && (strcmp(zipcode, high) <= 0)) {
        printf("%5s: %s\n", zipcode, city);
    }
}

void ListRecordsInRange(multitable *zipCodes, char *low, char *high) {
    char *endpoints[] = {low, high};
    MultiTableMap(zipCodes, InRangePrint, endpoints);
}

int main(int argc, const char * argv[]) {
    multitable zipCodeDatabase;
    MultiTableNew(&zipCodeDatabase, 6 * sizeof(char), sizeof(char *),
                  100000, ZipCodeHash, ZipCodeCompare);
    
    char code[6] = {"08077"};
    code[5] = '\0';
    char *cities[3] = {"Cinnaminson, NJ", "Riverton, NJ", "Palmyra, NJ"};
    
    MultiTableEnter(&zipCodeDatabase, &code, &cities[0]);
    MultiTableEnter(&zipCodeDatabase, &code, &cities[1]);
    MultiTableEnter(&zipCodeDatabase, &code, &cities[2]);
    
    ListRecordsInRange(&zipCodeDatabase, "08076", "08078");
    
    return 0;
}
