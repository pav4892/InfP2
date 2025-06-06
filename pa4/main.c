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

// Range of collatz-calculations that should be done

long long minValueCollatz = 1;          // long long because for example values like 63728127 can scale up to almost a trillion which overflows regular ints and fucks everything up: https://www.dcode.fr/collatz-conjecture
long long maxValueCollatz = 100000000;  // long long is a bit overkill but whatever, just be save and performance isn't too important because it's the relation in which the performance stands between each other that matters

long long collatzSum = 0;

// Time measuremnt of sequentiell and parallel methods

float tSeq;
float tPar;

int currentNodeAmount = 0;

typedef struct node {
  int myInt;
  struct node * next;
} node;

typedef struct rangeForMultiThreadingStruct {
  long long startRange;
  long long endRange;
} rangeForMultiThreadingStruct;

typedef struct startWertLaengsteFolgePaarStruct {
  long long startWert;
  long long laengeFolge;
} startWertLaengsteFolgePaarStruct;


// ----------------------------------- Linked List code(start) -------------------------------

node * head = NULL;
node * current = NULL;

// ----------------------------------- Linked List code(end) ---------------------------------

_Atomic long long producerSum = 0;
_Atomic long long consumerSum = 0;

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
    }; 

    atomic_fetch_add(&collatzSum, collatzFolgenLaenge);       // Remove this for non-atomic unsafe add
    //collatzSum += collatzFolgenLaenge;                      // Add this for unsafe add

    //lokale maxsumme oder mutex
    if(collatzFolgenLaenge > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
      myStartWertLaengsteFolgePaarStruct.startWert = y;
      myStartWertLaengsteFolgePaarStruct.laengeFolge = collatzFolgenLaenge;
    };

    //printf("Wert: %lld --> Collatz-Length: %lld\n", y, collatzFolgenLaenge);        //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement 
  }
  
  return (void *)inRangeForMultiThreadingStruct;

};

node * addElementAtStart(node * headPre, int value) {
  
  atomic_fetch_add(&currentNodeAmount, 1);

  node * newNode = malloc(sizeof(node));
  newNode->myInt = value;
  newNode->next = headPre;
  return newNode;
};

void removeTail(node * head) {

  atomic_fetch_sub(&currentNodeAmount, 1);

  node * currHead;
  pthread_mutex_lock(&myMutex1);
  while(head->next != NULL) {
    currHead = head;
    head = head->next;
  };

  head = NULL;
  free(head);
  currHead->next = NULL;
  pthread_mutex_unlock(&myMutex1);

};


void * producerFunction(void * args) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts); 
  srand(ts.tv_nsec);
  long long randomNumber = rand() % 1000; // Get a random number and make it less digits because I dont want to deal with overflow shit

  int myCounter = 0;

  // Jeder Producer erzeugt 1000 Zufallszahlen
  for(int j = 0; j < 100; j++) {

    //sem_wait(&empty);

    atomic_fetch_add(&producerSum, randomNumber);
    pthread_mutex_lock(&myMutex1);
    head = addElementAtStart(head, randomNumber);
    pthread_mutex_unlock(&myMutex1);
    sem_post(&full);  // Notify consumers
  };


  atomic_fetch_add(&myCounter, 5);
};


void * consumerFunction(void * args) {
  sem_wait(&full);  // Wait for avaialable

  long long randomNumber;

    //head = head->next;
  

  while(1) {
    //printf("|\n%d|\n", currentNodeAmount);
    if(currentNodeAmount > 0) {
      sem_post(&empty);
      //printf("\n|%d|\n", currentNodeAmount);
      atomic_fetch_add(&consumerSum, head->myInt);
      removeTail(head);
    } else {
      break;
    }
  }
  
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

    collatzSum += collatzFolgenLaenge;                  // Sequentiel code so no risk

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

  printf("\nCollatz-Summe: %llu\n\n", collatzSum);

  printf("----> Took %f seconds\n------------------------------------\n\n", tSeq);

};

void parallelCalc() {

  sem_init(&full, 0, 0);

  // Create a pre-defined amount of producer- and consumer threads. The producer thread creates random numbers and the consumer thread receives them. Sum up these random numbers in the producer and consumer and check if equal.
  // We check by summing up the random numbers before and after writing them to linked list. This equal check also has to be stress-tested with 50 producers(each Calculating 10000) and 30 consumers
 
  int amountProducerFunctionThreads = 50;
  int amountConsumerFunctionThreads = 30;

  // The linked list here is used as the shared buffer between the producer and consumer threads and I think that the PRODUCER has to fill it with random numbers and the CONSUMER has to use and then delete
  // The CONSUMER uses(or at least prints(know for sure) ) the random numbers it gets from the PRODUCER by doing some collatz stuff.
  // The functions to add and remove are the regular linked list function I have already implemented above
  
  //End goal(I think): Create a random number pass it to the producer thread(Make it accessible for consumer) that does the sum up for checksum. Then the consumer function takes that random number and does
  //the collatz-stuff and sum up for checksum. ALl this somehow synchronised via semaphore stuff(the linked list ig??)  
  
  pthread_t producerThread[amountProducerFunctionThreads]; 
  uintptr_t threadRetParam1 = -1;
  
  for(int i = 0; i < amountProducerFunctionThreads; i++) {
    pthread_create(&producerThread[i], NULL, &producerFunction, NULL); 
  };

  for(int i = 0; i < amountProducerFunctionThreads; i++) {
    pthread_join(producerThread[i], (void *)(&threadRetParam1));
  };

  pthread_t consumerThread[amountConsumerFunctionThreads]; 

  for(int i = 0; i < amountConsumerFunctionThreads; i++) {
    pthread_create(&consumerThread[i], NULL, &consumerFunction, NULL); 
  };

  for(int i = 0; i < amountConsumerFunctionThreads; i++) {
    pthread_join(consumerThread[i], (void *)(&threadRetParam1)); 
  };

  // Producers produce some elements that the Consumers consume and the program exits.

  int timeBeforeRun;
  int timeAfterRun;
  int timeBeforeRunNanos;
  int timeAfterRunNanos;

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  timeBeforeRun = ts.tv_sec;
  timeBeforeRunNanos = ts.tv_nsec;

  int threads = 16;
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

  printf("\n\nAmount of Nodes in list: %d\nProducer Sum: %lld\nConsumer Sum: %lld\n", currentNodeAmount, producerSum, consumerSum);

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

  float diagramY_speedup[24];

  printf("\n\n------------------------------------\n\nRunning Calcualtion of Speedup values for 2-24 threads and creating Diagram\n\n");

  for(int threads = 2; threads <= 24; threads++) {

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
  int diagramX_threads[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24}; 

  for (int i = 0; i < 23; i++) {
    fprintf(gnuplot, "%d %f\n", diagramX_threads[i], diagramY_speedup[i+2]);
  };

  fprintf(gnuplot, "e\n");

  pclose(gnuplot); // end process
};

int main() {

  sem_init(&empty, 0, 5);

  // Goal, Calculate Collatz-Folge for every value from 1-100.000.000
  
  myStartWertLaengsteFolgePaarStruct.startWert = 0;
  myStartWertLaengsteFolgePaarStruct.laengeFolge = 0;

  // Sequentiell

  //sequentialCalc();  // -- works
  
  collatzSum = 0;

  // Parallel
  
  parallelCalc(); // time measurment is done on a thread level
  
  printf("\n\nSpeed-up factor: %f\n\n", tSeq/tPar);
  printf("Die laengste Collatz-Folge(%lld-%lld) ist %lld lang und wird durch den Startwert %lld erzeugt\n", minValueCollatz, maxValueCollatz, myStartWertLaengsteFolgePaarStruct.laengeFolge, myStartWertLaengsteFolgePaarStruct.startWert);

  printf("\nCollatz-Summe: %llu\n", collatzSum);

  //speedupDiagram();

   return 0;
};
