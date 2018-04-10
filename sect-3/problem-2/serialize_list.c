#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LEN 128

typedef struct node {
    struct node *next;
    char word[MAX_WORD_LEN];
} node;

void printList(node *list) {
    node *curList = list;
    while (curList != NULL) {
        printf("%s\n", curList->word);
        
        curList = curList->next;
    }
}

void appendToList(node **list, char *str) {
    node *s = malloc(sizeof(node *) + sizeof(char) * MAX_WORD_LEN);
    s->next = *list;
    strcpy(s->word, str);
    
    *list = s;
}

int *serializeList(const void *list) {
    int *sl = malloc(sizeof(int));
    *sl = 0;
    int length = 0;
    
    const void **current = (const void **)list;
    char *str = NULL;
    int len = 0;
    while (current != NULL) {
        str = (char *)current + sizeof(int *);
        len = (int)strlen(str);
        //printf("%d %s\n", len, str);
        
        sl = realloc(sl, sizeof(int) + (length + len + 1) * sizeof(char));
        char *dest = (char *)sl + sizeof(int) + length * sizeof(char);
        strcpy(dest, str);
        
        length += len + 1;
        *sl += 1;
        
        current = (const void **)*current;
    }
    
    return sl;
}

int main(int argc, const char * argv[]) {
    node *list = NULL;
    appendToList(&list, "Purple");
    appendToList(&list, "Green");
    appendToList(&list, "Pink");
    appendToList(&list, "Yellow");
    appendToList(&list, "Red");
    
    printList(list);
    
    int *serialList = serializeList(list);
    
    printf("Serialized list of %d strings\n", *serialList);
    return 0;
}
