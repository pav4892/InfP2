#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

// Range of collatz-calculations that should be done

long long minValueCollatz = 1;          // long long because for example values like 63728127 can scale up to almost a trillion which overflows regular ints and fucks everything up: https://www.dcode.fr/collatz-conjecture
long long maxValueCollatz = 100000000;  // long long is a bit overkill but whatever, just be save and performance isn't too important because it's the relation in which the performance stands between each other that matters

// Time measuremnt of sequentiell and parallel methods

float tSeq;
float tPar;

typedef struct rangeForMultiThreadingStruct {
  long long startRange;
  long long endRange;
} rangeForMultiThreadingStruct;

typedef struct startWertLaengsteFolgePaarStruct {
  long long startWert;
  long long laengeFolge;
} startWertLaengsteFolgePaarStruct;

startWertLaengsteFolgePaarStruct myStartWertLaengsteFolgePaarStruct;

void *threadCalcSpeedup(void *args) {

  rangeForMultiThreadingStruct * inRangeForMultiThreadingStruct = (rangeForMultiThreadingStruct *)args;

  for(long long y = inRangeForMultiThreadingStruct->startRange; y <= inRangeForMultiThreadingStruct->endRange; y++) {
    long long collatzFolgenLaenge = 0;
    long long x = y;
    while(x > 1) {
      if(x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      }
      collatzFolgenLaenge += 1;
    } 

    if(collatzFolgenLaenge > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
      myStartWertLaengsteFolgePaarStruct.startWert = y;
      myStartWertLaengsteFolgePaarStruct.laengeFolge = collatzFolgenLaenge;
    }

    //printf("Wert: %lld --> Collatz-Length: %lld\n", y, collatzFolgenLaenge);        //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement 
  }
  
  return (void *)inRangeForMultiThreadingStruct;

};

void sequentialCalc() {

  printf("\n------------------------------------\nRunning sequential Collatz-Calculation...\n\n");
  fflush(stdout);    // idk for some reason I need this as if I don't include it it will just wait for the second print in this function for this to print which is... too late
 
  int timeBeforeRun;
  int timeAfterRun;
  int timeBeforeRunNanos;
  int timeAfterRunNanos;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  timeBeforeRun = ts.tv_sec;
  timeBeforeRunNanos = ts.tv_nsec;

  for(long long i = minValueCollatz; i <= maxValueCollatz; i++) {
    long long x = i;
    long long collatzFolgenLaenge = 0;
    while(x > 1) {
      if(x % 2 == 0) {
        x = x / 2;
      } else {
        x = 3 * x + 1;
      };

      collatzFolgenLaenge += 1;
    };

    if(collatzFolgenLaenge > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
      myStartWertLaengsteFolgePaarStruct.startWert = i;
      myStartWertLaengsteFolgePaarStruct.laengeFolge = collatzFolgenLaenge;
    };

    //printf("Wert: %lld --> Collatz-Length: %lld\n", i, collatzFolgenLaenge);    //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement
  }

  clock_gettime(CLOCK_REALTIME, &ts);
  timeAfterRun = ts.tv_sec;
  timeAfterRunNanos = ts.tv_nsec;

  tSeq = (timeAfterRun-timeBeforeRun)+((timeAfterRunNanos-timeBeforeRunNanos)/1e9);

  printf("----> Took %f seconds\n------------------------------------\n\n", tSeq);

};

void parallelCalc() {

  int timeBeforeRun;
  int timeAfterRun;
  int timeBeforeRunNanos;
  int timeAfterRunNanos;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  timeBeforeRun = ts.tv_sec;
  timeBeforeRunNanos = ts.tv_nsec;

  int threads = 8;
  int rangeSize = (maxValueCollatz-minValueCollatz)/threads;
  int rangeStepCounter = minValueCollatz - 1;
  pthread_t thread[threads];
  uintptr_t threadRetParam = -1;
  rangeForMultiThreadingStruct currentRange[threads];

  printf("\n------------------------------------\nRunning parallel Collatz-Calculation using %d threads...\n\n", threads);

  int timeBeforeRunThreads[threads];
  int timeAfterRunThreads[threads];
  float timeBeforeRunThreadsNanos[threads];
  float timeAfterRunThreadsNanos[threads];


  for(int i = 0; i < threads; i++) {
    struct timespec tsThread[threads];
    clock_gettime(CLOCK_REALTIME, &tsThread[i]);
    timeBeforeRunThreads[i] = tsThread[i].tv_sec;
    timeBeforeRunThreadsNanos[i] = tsThread[i].tv_nsec; // This will NOT get the nanos since epoch but instead get the nanos of the current second and it overflows and loops araound again after each second so we have to do math
 
    currentRange[i].startRange = rangeStepCounter + 1;
    rangeStepCounter += rangeSize;                  // Fucking kill me wtf is this code
    if(i == threads - 1) {                          // Need to to a seperate treatment because with ints you cant evenly distribute every number for every amount of threads, so we just give the last thread the rest
      currentRange[i].endRange = maxValueCollatz;
    } else {
      currentRange[i].endRange = rangeStepCounter;
    }

    pthread_create(&thread[i], NULL, &threadCalcSpeedup, &currentRange[i]); // Create a new thread and pass the fitting range to it 
  }

  for(int i = 0; i < threads; i++) {
    struct timespec tsThread[threads];
    pthread_join(thread[i], (void *)(&threadRetParam)); 
    clock_gettime(CLOCK_REALTIME, &tsThread[i]);
    timeAfterRunThreads[i] = tsThread[i].tv_sec;
    timeAfterRunThreadsNanos[i] = tsThread[i].tv_nsec; // This will NOT get the nanos since epoch but instead get the nanos of the current second and it overflows and loops araound again after each second so we have to do math
    printf("Thread %d finished after %f seconds and calculated the collatz sequence for the range: %lld-%lld - (%lld iterations)\n", i, ((timeAfterRunThreads[i]+(timeAfterRunThreadsNanos[i]/1e9))-(timeBeforeRunThreads[i]+(timeBeforeRunThreadsNanos[i]/1e9))), currentRange[i].startRange, currentRange[i].endRange, currentRange[i].endRange - currentRange[i].startRange);

  }

  clock_gettime(CLOCK_REALTIME, &ts);
  timeAfterRun = ts.tv_sec;
  timeAfterRunNanos = ts.tv_nsec;
  tPar = (timeAfterRun-timeBeforeRun)+((timeAfterRunNanos-timeBeforeRunNanos)/1e9);
  printf("\n\n----> Took %f seconds\n------------------------------------\n", tPar);
  
};

void speedupDiagram() {

  int timeBeforeRun;
  int timeAfterRun;
  int timeBeforeRunNanos;
  int timeAfterRunNanos;

  float diagramY_speedup[16];

  printf("\n\n------------------------------------\n\nRunning Calcualtion of Speedup values for 2-16 threads and creating Diagram\n\n");

  for(int threads = 2; threads <= 16; threads++) {

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    timeBeforeRun = ts.tv_sec;
    timeBeforeRunNanos = ts.tv_nsec;

    int rangeSize = (maxValueCollatz-minValueCollatz)/threads;
    int rangeStepCounter = minValueCollatz - 1;
    pthread_t thread[threads];
    uintptr_t threadRetParam = -1;
    rangeForMultiThreadingStruct currentRange[threads];

    int timeBeforeRunThreads[threads];
    int timeAfterRunThreads[threads];
    float timeBeforeRunThreadsNanos[threads];
    float timeAfterRunThreadsNanos[threads];


    for(int i = 0; i < threads; i++) {
      struct timespec tsThread[threads];
      clock_gettime(CLOCK_REALTIME, &tsThread[i]);
      timeBeforeRunThreads[i] = tsThread[i].tv_sec;
      timeBeforeRunThreadsNanos[i] = tsThread[i].tv_nsec; // This will NOT get the nanos since epoch but instead get the nanos of the current second and it overflows and loops araound again after each second so we have to do math
  
      currentRange[i].startRange = rangeStepCounter + 1;
      rangeStepCounter += rangeSize;                  // Fucking kill me wtf is this code
      if(i == threads - 1) {                          // Need to to a seperate treatment because with ints you cant evenly distribute every number for every amount of threads, so we just give the last thread the rest
        currentRange[i].endRange = maxValueCollatz;
      } else {
        currentRange[i].endRange = rangeStepCounter;
      }

      pthread_create(&thread[i], NULL, &threadCalcSpeedup, &currentRange[i]); // Create a new thread and pass the fitting range to it 
    }

    for(int i = 0; i < threads; i++) {
      struct timespec tsThread[threads];
      pthread_join(thread[i], (void *)(&threadRetParam)); 
      clock_gettime(CLOCK_REALTIME, &tsThread[i]);
      timeAfterRunThreads[i] = tsThread[i].tv_sec;
      timeAfterRunThreadsNanos[i] = tsThread[i].tv_nsec; // This will NOT get the nanos since epoch but instead get the nanos of the current second and it overflows and loops araound again after each second so we have to do math
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    timeAfterRun = ts.tv_sec;
    timeAfterRunNanos = ts.tv_nsec;
    tPar = (timeAfterRun-timeBeforeRun)+((timeAfterRunNanos-timeBeforeRunNanos)/1e9);

    diagramY_speedup[threads] = tSeq/tPar;

    printf("----> %d Threads: Took %f seconds - Speedup: %f\n", threads, tPar, tSeq/tPar);
  }; 

  FILE *gnuplot = popen("gnuplot -persistent", "w");  // Calls the linux package `gnuplot` via bash call
  
  fprintf(gnuplot, "set xlabel 'Threads'\n");
  fprintf(gnuplot, "set ylabel 'Speedup'\n");

  
  fprintf(gnuplot, "plot '-' with lines\n"); // Draws the points and connects them with a line through all of themn. frptintf passes it to the process I give it
  int diagramX_threads[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}; 

  for (int i = 0; i < 15; i++) {
    fprintf(gnuplot, "%d %f\n", diagramX_threads[i], diagramY_speedup[i+2]);
  };

  fprintf(gnuplot, "e\n");

  pclose(gnuplot); // end process
};

int main() {

  // Goal, Calculate Collatz-Folge for every value from 1-100.000.000
  
  myStartWertLaengsteFolgePaarStruct.startWert = 0;
  myStartWertLaengsteFolgePaarStruct.laengeFolge = 0;

  
  // Sequentiell

  sequentialCalc();  // -- works

  // Parallel

  parallelCalc(); // time measurment is done on a thread level
  
  printf("\n\nSpeed-up factor: %f\n\n", tSeq/tPar);
  printf("Die laengste Collatz-Folge(%lld-%lld) ist %lld lang und wird durch den Startwert %lld erzeugt\n", minValueCollatz, maxValueCollatz, myStartWertLaengsteFolgePaarStruct.laengeFolge, myStartWertLaengsteFolgePaarStruct.startWert);

  speedupDiagram();
  
  return 0;
}
