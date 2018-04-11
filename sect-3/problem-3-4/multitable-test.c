#include <stdio.h>
#include <string.h>
#include "multitable.h"

static int ZipCodeHash(const void *keyAddr, int numBuckets) {
    return 0;
}

static int ZipCodeCompare(const void *keyAddr1, const void *keyAddr2) {
    return 0;
}

static void InRangePrint(void *keyAddr, void *valueAddr, void *auxData) {
    char *zipcode;
    char *city;
    char *low;
    char *high;
    
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
    char *cities[3] = {"Cinnaminson, NJ", "Cinnaminson, NJ", "Palmyra, NJ"};
    
    MultiTableEnter(&zipCodeDatabase, &code, cities[0]);
    MultiTableEnter(&zipCodeDatabase, &code, cities[1]);
    MultiTableEnter(&zipCodeDatabase, &code, cities[2]);
    
    ListRecordsInRange(&zipCodeDatabase, "08076", "08078");
    
    return 0;
}
