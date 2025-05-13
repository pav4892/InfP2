#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

typedef struct rangeForMultiThreadingStruct {
  int startRange;
  int endRange;
} rangeForMultiThreadingStruct;

void *threadCalcSpeedup(void *args) {

  int collatzFolgenLaenge = 0; // starts at 1 because the inital value of 11 is part of the Folge

  rangeForMultiThreadingStruct * inRangeForMultiThreadingStruct = (rangeForMultiThreadingStruct *)args;

  printf("------------------\nI am a thread and currently working on range: %d-%d\n", inRangeForMultiThreadingStruct->startRange,inRangeForMultiThreadingStruct->endRange);
  
  int x = inRangeForMultiThreadingStruct->startRange;
  
//  for(int x = inRangeForMultiThreadingStruct->startRange; x <= inRangeForMultiThreadingStruct->endRange; x++) {
    while(x > 1) {
      if(x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      }
      collatzFolgenLaenge += 1;
    }
    printf("Wert: %d --> Collatz-Length: %d\n--------------------", inRangeForMultiThreadingStruct->startRange, collatzFolgenLaenge);
 // }
  
  return (void *)inRangeForMultiThreadingStruct;

};

int main() {
  // Goal, Calculate Collatz-Folge for every value from 1-100.000.000
 
  int maxValueCollatz = 100000000;
  int threads = 100;
  int rangeSize = maxValueCollatz/threads;
  int rangeStepCounter = 0;
  pthread_t thread;
  uintptr_t threadRetParam = -1;

  for(int i = 0; i < threads; i++) {
    
    rangeForMultiThreadingStruct currentRange;
    currentRange.startRange = rangeStepCounter;
    rangeStepCounter += rangeSize;                  // Fucking kill me wtf is this code
    currentRange.endRange = rangeStepCounter;

    pthread_create(&thread, NULL, &threadCalcSpeedup, &currentRange); // Create a new thread and pass the fitting range to it
    pthread_join(thread, (void *)(&threadRetParam));
  }

  return 0;
}
