#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

long long minValueCollatz = 1;          // long long because for example values like 63728127 can scale up to almost a trillion which overflows regular ints and fucks everything up: https://www.dcode.fr/collatz-conjecture
long long maxValueCollatz = 100000000;  // long long is a bit overkill but whatever, just be save and performance isn't too important because it's the relation in which the performance stands between each other that matters

float tSeq;
float tPar;

typedef struct startWertLaengsteFolgePaarStruct {
  long long startWert;
  long long laengeFolge;
} startWertLaengsteFolgePaarStruct;

startWertLaengsteFolgePaarStruct myStartWertLaengsteFolgePaarStruct;

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

    //printf("%d\n", collatzFolgenLaenge);

    if(collatzFolgenLaenge > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
      myStartWertLaengsteFolgePaarStruct.startWert = i;
      myStartWertLaengsteFolgePaarStruct.laengeFolge = collatzFolgenLaenge;
    };

    //printf("Wert: %d --> Collatz-Length: %d\n", i, collatzFolgenLaenge);    //optionale Ausgabe -- stdout is bottleneck in speed so don't do this for time measurement
  }

  clock_gettime(CLOCK_REALTIME, &ts);
  timeAfterRun = ts.tv_sec;
  timeAfterRunNanos = ts.tv_nsec;

  tSeq = (timeAfterRun-timeBeforeRun)+((timeAfterRunNanos-timeBeforeRunNanos)/1e9);

  printf("----> Took %f seconds\n------------------------------------\n\n", tSeq);

};

int main() {

  // Goal, Calculate Collatz-Folge for every value from 1-100.000.000
  
  myStartWertLaengsteFolgePaarStruct.startWert = 0;
  myStartWertLaengsteFolgePaarStruct.laengeFolge = 0;

  
  // Sequentiell

  sequentialCalc();  // -- works

  printf("Die laengste Collatz-Folge(%d-%d) ist %d lang und wird durch den Startwert %d erzeugt", minValueCollatz, maxValueCollatz, myStartWertLaengsteFolgePaarStruct.laengeFolge, myStartWertLaengsteFolgePaarStruct.startWert);
  
  return 0;
}

