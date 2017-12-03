#include <iostream>
#include <stdlib.h>
using namespace std;

typedef enum {
  Integer, String, List, Nil
} nodeType;

nodeType *allocIntNode(int number) {
  nodeType *intNode = (nodeType *) malloc(sizeof(nodeType) + sizeof(int));
  *intNode = Integer;
  *(int *)((char *)intNode + sizeof(nodeType)) = number;
  return intNode;
}

nodeType *allocStrNode(char *str) {
  nodeType *strNode = (nodeType *) malloc(sizeof(nodeType) + sizeof(char) * (strlen(str) + 1));
  *strNode = String;
  strcpy((char *)strNode + sizeof(nodeType), str);
  return strNode;
}

nodeType *allocListNode(nodeType *current, nodeType *next) {
  nodeType *listNode = (nodeType *) malloc(sizeof(nodeType) + sizeof(nodeType *) * 2);
  *listNode = List;
  memcpy((char *)listNode + sizeof(nodeType), &current, sizeof(nodeType *));
  memcpy((char *)listNode + sizeof(nodeType) + sizeof(nodeType *), &next, sizeof(nodeType *));
  return listNode;
}

nodeType *allocNilNode() {
  nodeType *nilNode = (nodeType *) malloc(sizeof(nodeType));
  *nilNode = Nil;
  return nilNode;
}

void printNode(nodeType *node) {
  nodeType type = *node;
  void *content = (char *)node + sizeof(nodeType);
  switch(type) {
  case Integer: {
    cout << *(int *)content;
    break;
  }
  case String: {
    cout << (char *)content;
    break;
  }
  case List: {
    nodeType *current = *(nodeType **)content;
    nodeType *next = *(nodeType **)((char *)content + sizeof(nodeType *));
    cout << "(";
    printNode(current);
    printNode(next);
    cout << ")";
    break;
  }
  case Nil: {
    break;
  }
  }
}

int main() {  
  nodeType *nil = allocNilNode();

  nodeType *firstInt = allocIntNode(1);
  nodeType *list1 = allocListNode(firstInt, nil);

  nodeType *firstStr = allocStrNode("Diamondbacks");
  nodeType *list2 = allocListNode(firstStr, list1);

  nodeType *secondInt = allocIntNode(2);
  nodeType *list3 = allocListNode(secondInt, list2);

  nodeType *secondStr = allocStrNode("Yankees");
  nodeType *gameThree = allocListNode(secondStr, list3);
  
  printNode(gameThree);
  cout << endl;

  return 0;
}
