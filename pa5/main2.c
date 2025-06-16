#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>        
#include <fcntl.h>           
#include <unistd.h>         // This introduces a sync which blocks me from using the global struct I am trying to define below because it was originally called sync aswell and that casues a conflict so now it's called syncStruct

int iterations = 50;

typedef struct syncStructStruct {
    sem_t newWork;
    sem_t newResult;
    void * payloadOffset;
} syncStruct;

typedef struct payloadStruct {
    int zahl;
    int exponent;
    int result;
} payloadStruct;

syncStruct *sharedMemorySpacePointer;
payloadStruct myPayloadStruct;

void *producerFunction(void *args) {

    int sharedMemorySpace = shm_open("/mySharedMemSpace", O_CREAT | O_RDWR, 0666); 
    ftruncate(sharedMemorySpace , 65535);

    void *addr = mmap(0, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemorySpace, 0);

    sharedMemorySpacePointer = (syncStruct *)addr;

    
    sem_init(&sharedMemorySpacePointer->newWork, 1, 0);
    sem_init(&sharedMemorySpacePointer->newResult, 1, 5);

    for(int i = 0; i < iterations; i++) { 
        sharedMemorySpacePointer->payloadOffset = (void *)(intptr_t)8;
        sem_post(&sharedMemorySpacePointer->newWork);
    };

    return NULL;
}

void *consumerFunction(void *args) {
    
    while (sharedMemorySpacePointer == NULL) {
        usleep(1000);
    }

    printf("%ld", (long)sharedMemorySpacePointer->payloadOffset);

    return NULL;
}

int main() {
    srand(time(NULL));

    pthread_t producer;
    pthread_t consumer; 
 
    pthread_create(&producer, NULL, producerFunction, NULL);
    pthread_create(&consumer, NULL, consumerFunction, NULL);

    pthread_join(producer, NULL);
    sem_post(&sharedMemorySpacePointer->newResult); 
    pthread_join(consumer, NULL);

    // Cleanup
    sem_destroy(&sharedMemorySpacePointer->newResult);
    sem_destroy(&sharedMemorySpacePointer->newWork);

    return 0;
}

