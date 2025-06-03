#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int currentNodeAmount = 0;

typedef struct node {
  int myInt;
  struct node * next;
} node;

node * addElementAtStart(node * headPre, int value) {
  
  currentNodeAmount += 1;

  node * newNode = malloc(sizeof(node));
  newNode->myInt = value;
  newNode->next = headPre;
  return newNode;
};

void removeTail(node * head) {

  currentNodeAmount -= 1;

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

  node * current = head;
  while (current != NULL) {
    printf("%d -> ", current->myInt);
    current = current->next;
  }
  printf("NULL\n");
}

int main() {


  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts); 

  srand(ts.tv_nsec); // Initialize random number generator with current amount of nanos since last second

  int nodesAmount = 16;

  node * head = NULL;
  node * current = NULL;

  for(int i = 0; i < nodesAmount; i++) {

    currentNodeAmount += 1; 

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
  

  printList(head);

  return 0;
};

