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
    if (list == NULL) {
        *sl = 0;
        return sl;
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
    
    printf("Hello, World!\n");
    return 0;
}
