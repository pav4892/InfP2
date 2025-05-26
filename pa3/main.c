#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct argsStruct {    // Need a struct because we want to pass more than 1 argument to the thread function in the interest of code consicenss and quality
  int iterations; // needs to be more than 1000 or something higher in general because else thread 1 will spawn and finish before the second thread even starts which does not produce the expected race condition
  int counterChangeDirection; // so we dont need seperate functions for increment/decrment
} argsStruct;

int counter = 0;

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t mySemaphore;

void *unsafeCounterThreadFunc(void *args) {
  argsStruct * myArgsStruct = (argsStruct *)args;
  int iterations = myArgsStruct->iterations;

  for(int i = 0; i < iterations; i++) {
    counter += myArgsStruct->counterChangeDirection;
  };
};

void *safeMutexCounterThreadFunc(void *args) {
  argsStruct * myArgsStruct = (argsStruct *)args;
  int iterations = myArgsStruct->iterations;

  for(int i = 0; i < iterations; i++) {
    pthread_mutex_lock(&myMutex);
    counter += myArgsStruct->counterChangeDirection;
    pthread_mutex_unlock(&myMutex);
  };
};

void *safeSemaphoreCounterThreadFunc(void *args) {
  argsStruct * myArgsStruct = (argsStruct *)args;
  int iterations = myArgsStruct->iterations;

  for(int i = 0; i < iterations; i++) {
    sem_wait(&mySemaphore);
    counter += myArgsStruct->counterChangeDirection;
    sem_post(&mySemaphore);
  };
};

void multiThreadCounterChange(void *(*counterThreadFunc)(void *)) {

  argsStruct threadFunctionArgsStructIncrement;
  argsStruct threadFunctionArgsStructDecrement;

  threadFunctionArgsStructIncrement.iterations = 10000000;
  threadFunctionArgsStructIncrement.counterChangeDirection = 1;

  threadFunctionArgsStructDecrement.iterations = 10000000;
  threadFunctionArgsStructDecrement.counterChangeDirection = -1;

  pthread_t thread1;
  pthread_t thread2;

  pthread_create(&thread1, NULL, counterThreadFunc, &threadFunctionArgsStructIncrement);
  pthread_create(&thread2, NULL, counterThreadFunc, &threadFunctionArgsStructDecrement);

  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  printf("%d\n", counter);

};

int main() {

  // We decide which way we want to manipulate the pointer by passing in the names of the thread functions that are called later because the mutex/samaphore or normal stuff is done there

  printf("Unsafe Counter Change: ");
  multiThreadCounterChange(unsafeCounterThreadFunc);

  counter = 0;

  printf("Safe Counter Change using Mutex: ");
  multiThreadCounterChange(safeMutexCounterThreadFunc);

  counter = 0;

  sem_init(&mySemaphore, 0, 1);  // , Processintern, startvalue: 1   | DO NOT PLACE THIS INSIDE OF THE THREAD FUNCTION IT WILL BREAK IDK WHY
  printf("Safe Counter Change using Semaphore: ");
  multiThreadCounterChange(safeSemaphoreCounterThreadFunc);

  return 0;
}

