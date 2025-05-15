#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

typedef struct rangeForMultiThreadingStruct {
  int startRange;
  int endRange;
} rangeForMultiThreadingStruct;

void *threadCalcSpeedup(void *args) {

  rangeForMultiThreadingStruct * inRangeForMultiThreadingStruct = (rangeForMultiThreadingStruct *)args;

  for(int y = inRangeForMultiThreadingStruct->startRange; y <= inRangeForMultiThreadingStruct->endRange; y++) {
    int collatzFolgenLaenge = 0;
    int x = y;
    while(x > 1) {
      if(x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      }
      collatzFolgenLaenge += 1;
    } 

    //printf("Wert: %d --> Collatz-Length: %d\n", y, collatzFolgenLaenge);        //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement
  }
  
  return (void *)inRangeForMultiThreadingStruct;

};

void sequentialCalc() {

  printf("\n------------------------------------\nRunning sequential Collatz-Calculation...\n\n");
  fflush(stdout);    // idk for some reason I need this as if I don't include it it will just wait for the second print in this function for this to print which is... too late
 
  int timeBeforeRun;
  int timeAfterRun;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  timeBeforeRun = ts.tv_sec;

  for(int i = 0; i <= 100000000; i++) {
    int x=i;
    int collatzFolgenLaenge = 0;
    while(x > 1) {
      if(x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      };

      collatzFolgenLaenge += 1;
    }

    //printf("Wert: %d --> Collatz-Length: %d\n", i, collatzFolgenLaenge);    //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement
  }

  clock_gettime(CLOCK_REALTIME, &ts);
  timeAfterRun = ts.tv_sec;
  printf("Took about %d seconds\n------------------------------------\n\n", (timeAfterRun-timeBeforeRun));

};

void parallelCalc() {


  int timeBeforeRun;
  int timeAfterRun;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  timeBeforeRun = ts.tv_sec;

  int minValueCollatz = 1;
  int maxValueCollatz = 100000000;
  int threads = 16;
  int rangeSize = (maxValueCollatz-minValueCollatz)/threads;
  int rangeStepCounter = 0;
  pthread_t thread[threads];
  uintptr_t threadRetParam = -1;
  rangeForMultiThreadingStruct currentRange[threads];

  printf("\n------------------------------------\nRunning parallel Collatz-Calculation using %d threads...\n\n", threads);

  int timeBeforeRunThreads[threads];
  int timeAfterRunThreads[threads];

  for(int i = 0; i < threads; i++) {
    struct timespec tsThread[threads];
    clock_gettime(CLOCK_REALTIME, &tsThread[i]);
    timeBeforeRunThreads[i] = tsThread[i].tv_sec;
 
    currentRange[i].startRange = rangeStepCounter + minValueCollatz;
    rangeStepCounter += rangeSize;                  // Fucking kill me wtf is this code
    currentRange[i].endRange = rangeStepCounter;

    pthread_create(&thread[i], NULL, &threadCalcSpeedup, &currentRange[i]); // Create a new thread and pass the fitting range to it 
  }

  for(int i = 0; i < threads; i++) {
    struct timespec tsThread[threads];
    pthread_join(thread[i], (void *)(&threadRetParam)); 
    clock_gettime(CLOCK_REALTIME, &tsThread[i]);
    timeAfterRunThreads[i] = tsThread[i].tv_sec;
    printf("Thread %d took %d seconds\n",i, (timeAfterRunThreads[i]-timeBeforeRunThreads[i]));
  }

  clock_gettime(CLOCK_REALTIME, &ts);
  timeAfterRun = ts.tv_sec;
  printf("Took about %d seconds\n------------------------------------\n", (timeAfterRun-timeBeforeRun));
  
}

int main() {

  // Goal, Calculate Collatz-Folge for every value from 1-100.000.000

  // Sequentiell

  sequentialCalc();  // -- works

  // Parallel

  parallelCalc(); // time measurment is done on a thread level
  
  return 0;
}
