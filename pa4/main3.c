#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdatomic.h>
#include <semaphore.h>

pthread_mutex_t myMutex1 = PTHREAD_MUTEX_INITIALIZER;
sem_t full;
sem_t empty;

long long minValueCollatz = 1;
long long maxValueCollatz = 100000000;

long long collatzSum = 0;
float tSeq;
float tPar;

_Atomic int currentNodeAmount = 0;

typedef struct node {
  int myInt;
  struct node *next;
} node;

typedef struct rangeForMultiThreadingStruct {
  long long startRange;
  long long endRange;
} rangeForMultiThreadingStruct;

typedef struct startWertLaengsteFolgePaarStruct {
  long long startWert;
  long long laengeFolge;
} startWertLaengsteFolgePaarStruct;

node *head = NULL;

_Atomic long long producerSum = 0;
_Atomic long long consumerSum = 0;
startWertLaengsteFolgePaarStruct myStartWertLaengsteFolgePaarStruct;

void *threadCalcSpeedup(void *args) {
  rangeForMultiThreadingStruct *range = (rangeForMultiThreadingStruct *)args;

  for (long long y = range->startRange; y <= range->endRange; y++) {
    long long len = 0, x = y;
    while (x > 1) {
      x = (x % 2 == 0) ? x / 2 : 3 * x + 1;
      len++;
    }

    atomic_fetch_add(&collatzSum, len);
    if (len > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
      myStartWertLaengsteFolgePaarStruct.startWert = y;
      myStartWertLaengsteFolgePaarStruct.laengeFolge = len;
    }
  }
  return NULL;
}

node *addElementAtStart(node *headPre, int value) {
  node *newNode = malloc(sizeof(node));
  newNode->myInt = value;
  newNode->next = headPre;
  atomic_fetch_add(&currentNodeAmount, 1);
  return newNode;
}

void removeTail() {
  pthread_mutex_lock(&myMutex1);
  if (!head) {
    pthread_mutex_unlock(&myMutex1);
    return;
  }
  node *prev = NULL, *curr = head;
  while (curr->next) {
    prev = curr;
    curr = curr->next;
  }
  if (prev)
    prev->next = NULL;
  else
    head = NULL;

  atomic_fetch_sub(&currentNodeAmount, 1);
  free(curr);
  pthread_mutex_unlock(&myMutex1);
}

void *producerFunction(void *args) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  srand(ts.tv_nsec);
  long long randomNumber = 5;

  int count = 0;
  while (count < 100) {
    for (int j = 0; j < 5; j++) {
      sem_wait(&empty);
      pthread_mutex_lock(&myMutex1);
      head = addElementAtStart(head, randomNumber);
      pthread_mutex_unlock(&myMutex1);
      atomic_fetch_add(&producerSum, randomNumber);
      sem_post(&full);
    }
    count += 5;
  }
  return NULL;
}

void *consumerFunction(void *args) {
  while (1) {
    //sem_wait(&full);
    pthread_mutex_lock(&myMutex1);
    if (currentNodeAmount > 0 && head) {
      consumerSum += head->myInt;
      removeTail();
      sem_post(&empty);
      pthread_mutex_unlock(&myMutex1);
    } else {
      pthread_mutex_unlock(&myMutex1);
      sem_post(&full); // allow other consumers to terminate cleanly
      break;
    }
  }
  return NULL;
}

void parallelCalc() {
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, 100);  // Make sure this matches your buffer capacity

  int amountProducerFunctionThreads = 50;
  int amountConsumerFunctionThreads = 30;

  pthread_t producers[amountProducerFunctionThreads];
  pthread_t consumers[amountConsumerFunctionThreads];

  for (int i = 0; i < amountProducerFunctionThreads; i++)
    pthread_create(&producers[i], NULL, producerFunction, NULL);
  for (int i = 0; i < amountProducerFunctionThreads; i++)
    pthread_join(producers[i], NULL);

  for (int i = 0; i < amountConsumerFunctionThreads; i++)
    pthread_create(&consumers[i], NULL, consumerFunction, NULL);
  for (int i = 0; i < amountConsumerFunctionThreads; i++)
    pthread_join(consumers[i], NULL);

  printf("\n\nAmount of Nodes in list: %d\nProducer Sum: %lld\nConsumer Sum: %lld\n",
         currentNodeAmount, producerSum, consumerSum);

  // Now perform multithreaded Collatz calculation
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  int timeBeforeRun = ts.tv_sec;
  int timeBeforeRunNanos = ts.tv_nsec;

  int threads = 16;
  int rangeSize = (maxValueCollatz - minValueCollatz) / threads;
  int rangeStepCounter = minValueCollatz - 1;
  pthread_t thread[threads];
  rangeForMultiThreadingStruct currentRange[threads];

  printf("\n------------------------------------\nRunning parallel Collatz-Calculation using %d threads...\n\n", threads);

  for (int i = 0; i < threads; i++) {
    currentRange[i].startRange = rangeStepCounter + 1;
    rangeStepCounter += rangeSize;
    currentRange[i].endRange = (i == threads - 1) ? maxValueCollatz : rangeStepCounter;
    pthread_create(&thread[i], NULL, threadCalcSpeedup, &currentRange[i]);
  }

  for (int i = 0; i < threads; i++)
    pthread_join(thread[i], NULL);

  clock_gettime(CLOCK_REALTIME, &ts);
  int timeAfterRun = ts.tv_sec;
  int timeAfterRunNanos = ts.tv_nsec;
  tPar = (timeAfterRun - timeBeforeRun) + ((timeAfterRunNanos - timeBeforeRunNanos) / 1e9);

  printf("\n\n----> Took %f seconds\n------------------------------------\n", tPar);
}

int main() {
  myStartWertLaengsteFolgePaarStruct.startWert = 0;
  myStartWertLaengsteFolgePaarStruct.laengeFolge = 0;

  parallelCalc();

  printf("\n\nSpeed-up factor: %f\n", (tSeq > 0 ? tSeq / tPar : 0));
  printf("Die laengste Collatz-Folge(%lld-%lld) ist %lld lang und wird durch den Startwert %lld erzeugt\n",
         minValueCollatz, maxValueCollatz,
         myStartWertLaengsteFolgePaarStruct.laengeFolge,
         myStartWertLaengsteFolgePaarStruct.startWert);

  printf("\nCollatz-Summe: %llu\n", collatzSum);

  return 0;
}

