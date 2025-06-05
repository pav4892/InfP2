#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <time.h>

typedef struct node {
    int myInt;
    struct node *next;
} node;

node * head;

pthread_mutex_t myMutex1 = PTHREAD_MUTEX_INITIALIZER;

sem_t full;
sem_t empty;

atomic_int currentNodeAmount = 0;
atomic_int producerSum = 0;
atomic_int consumerSum = 0;

void insertAtHead(int value) {
    node *newNode = malloc(sizeof(node));
    newNode->myInt = value;
    newNode->next = head;
    head = newNode;
    atomic_fetch_add(&currentNodeAmount, 1);
}

void removeTail() {
    if (!head) return;

    node *temp = head;
    node *prev = NULL;

    if (!temp->next) {
        atomic_fetch_add(&consumerSum, temp->myInt);
        head = NULL;
        free(temp);
        atomic_fetch_sub(&currentNodeAmount, 1);
        return;
    }

    while (temp->next) {
        prev = temp;
        temp = temp->next;
    }

    atomic_fetch_add(&consumerSum, temp->myInt);
    prev->next = NULL;
    free(temp);
    atomic_fetch_sub(&currentNodeAmount, 1);
}

void *producerFunction(void *arg) {
    for (int i = 0; i < 10000; i++) {
        int randVal = rand() % 10;

        sem_wait(&empty);
        pthread_mutex_lock(&myMutex1);

        insertAtHead(randVal);
        atomic_fetch_add(&producerSum, randVal);

        pthread_mutex_unlock(&myMutex1);
        sem_post(&full);
    }

    return NULL;
}

void *consumerFunction(void *arg) {
    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&myMutex1);

        if (currentNodeAmount > 0) { // only remove tail if it exists
            removeTail();
            if(currentNodeAmount > 5) {
                printf("\n\nMehr als 5 Elemente in der Liste!!! Illegal\n\n"); // passiert nie weil semaphore auf 5 initialisiert ist
                break;
            }
            pthread_mutex_unlock(&myMutex1);
            sem_post(&empty);
        } else {
            pthread_mutex_unlock(&myMutex1);
            sem_post(&full);
            break;
        }
    }

    return NULL;
}

void freeList() {
    node *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    srand(time(NULL));

    int numProducers = 50;
    int numConsumers = 30;

    pthread_t producers[numProducers];
    pthread_t consumers[numConsumers];

    sem_init(&full, 0, 0);
    sem_init(&empty, 0, 5); // at most 5 elem at a time
 
    for (int i = 0; i < numProducers; i++) {
        pthread_create(&producers[i], NULL, producerFunction, NULL);
    }

    for (int i = 0; i < numConsumers; i++) {
        pthread_create(&consumers[i], NULL, consumerFunction, NULL);
    }

    for (int i = 0; i < numProducers; i++) {
        pthread_join(producers[i], NULL);
    }

    for (int i = 0; i < numConsumers; i++) {
        sem_post(&full); // Wake up any remaining consumers by incrementing 1 to the internal semaphore counter for all consumers
    }

    for (int i = 0; i < numConsumers; i++) {
        pthread_join(consumers[i], NULL);
    }

    printf("Producer Sum: %d\n", producerSum);
    printf("Consumer Sum: %d\n", consumerSum);

    // Cleanup stuff
    free(head);
    freeList();
    pthread_mutex_destroy(&myMutex1);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}

