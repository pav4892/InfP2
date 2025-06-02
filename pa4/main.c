#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct node {
  int myInt;
  struct node * next;
} node;

node * addElementAtStart(node * headPre, int value) {
  node * newNode = malloc(sizeof(node));
  newNode->myInt = value;
  newNode->next = headPre;
  return newNode;
};

void removeTail(node * head) {
  node * currHead;
  while(head->next != NULL) {
    currHead = head;
    head = head->next;
  };

  head = NULL;
  currHead->next = NULL;

};

void printList(node * head) {
  printf("List: ");
  if (head == NULL) {
    printf("Empty\n");
    return;
  }

  node * current = head;
  while (current != NULL) {
    printf("%d -> ", current->myInt);
    current = current->next;
  }
  printf("NULL\n");
}

int main() {
  srand(time(NULL));

  int nodesAmount = 16;

  node * head = NULL;
  node * current = NULL;

  for(int i = 0; i < nodesAmount; i++) {
    node * newNode = malloc(sizeof(node));
    newNode->myInt = rand();
    newNode->next = NULL;

    if(head == NULL) {
      head = newNode;
    } else {
      current->next = newNode;
    };

    current = newNode; 
  };

  printList(head);

  head = addElementAtStart(head, 100); // add a new element at the beginning ( replace the first element )
  removeTail(head); // remove last element 
  //
  printf("\n\n");

  printList(head);

  return 0;
};

