#include <stdio.h>
#include <stdint.h>
#include <time.h>

uint64_t minValueCollatz = 1;
uint64_t maxValueCollatz = 100000000;

float tSeq;

typedef struct {
    uint64_t startWert;
    int laengeFolge;
} startWertLaengsteFolgePaarStruct;

startWertLaengsteFolgePaarStruct myStartWertLaengsteFolgePaarStruct;

void sequentialCalc() {

    printf("\n------------------------------------\nRunning sequential Collatz-Calculation...\n\n");
    fflush(stdout);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long timeBeforeRun = ts.tv_sec * 1e9 + ts.tv_nsec;

    for (uint64_t i = minValueCollatz; i <= maxValueCollatz; i++) {
        uint64_t x = i;
        int collatzFolgenLaenge = 1;

        while (x != 1) {
            if (x % 2 == 0) {
                x = x / 2;
            } else {
                x = 3 * x + 1;
            }
            collatzFolgenLaenge++;
        }

        if (collatzFolgenLaenge > myStartWertLaengsteFolgePaarStruct.laengeFolge) {
            myStartWertLaengsteFolgePaarStruct.startWert = i;
            myStartWertLaengsteFolgePaarStruct.laengeFolge = collatzFolgenLaenge;
        }
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    long timeAfterRun = ts.tv_sec * 1e9 + ts.tv_nsec;

    tSeq = (timeAfterRun - timeBeforeRun) / 1e9;
    printf("----> Took %f seconds\n------------------------------------\n\n", tSeq);
}

int main() {

    myStartWertLaengsteFolgePaarStruct.startWert = 0;
    myStartWertLaengsteFolgePaarStruct.laengeFolge = 0;

    sequentialCalc();

    printf("Die laengste Collatz-Folge(%llu-%llu) ist %d lang und wird durch den Startwert %llu erzeugt\n",
           minValueCollatz, maxValueCollatz,
           myStartWertLaengsteFolgePaarStruct.laengeFolge,
           myStartWertLaengsteFolgePaarStruct.startWert);

    return 0;
}

