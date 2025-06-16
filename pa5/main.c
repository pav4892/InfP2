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


int iterations = 50; // amount of times the producer and consumer should generate add and write back a number. 1 Iteration is the producer writing a random number to 'zahl' and a random exponent to 'exponent' and then
                     // the consumer comes in and does the math and writes the result to the shared memory space into the 'result'. 

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

syncStruct mySyncStruct;
syncStruct * sharedMemorySpacePointer;
payloadStruct myPayloadStruct;

void *producerFunction(void *args) {

    int sharedMemorySpace = shm_open("/mySharedMemSpace", O_CREAT | O_RDWR, 0666); 
    ftruncate(sharedMemorySpace , 65535);

    // Map the shared memory into the process's address space: here we create a space at address space 0 (or close) of size 65535 bytes with Read and Write, declare it as shared and push the shared space we created before to it
    sharedMemorySpacePointer = mmap(0, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemorySpace, 0);

    sem_init(&mySyncStruct.newWork, 1, 0);   // the second parameter being non-zero means that the semaphore can be used by multiple prozesses not just intern of the thread. Initialized to 0
    sem_init(&mySyncStruct.newResult, 1, 5); // at most 5 elem at a time. Initialized to 5
    

    //Loop from here, semaphores can be reused indefinetly, payloadOffsetz zahl exponent are updated here and result in the consumer. I think the syncStruct remains at the start of the shared memory space
    
    for(int i = 0; i < iterations; i++) { 
        printf("f\n");
        mySyncStruct.payloadOffset = 0x00;
        *sharedMemorySpacePointer = mySyncStruct;
        sem_post(&sharedMemorySpacePointer->newWork);
    };
    
};

void *consumerFunction(void *args) {
    printf("%d", sharedMemorySpacePointer->payloadOffset);
};

int main() {
    srand(time(NULL));

    pthread_t producer;
    pthread_t consumer; 
 
    pthread_create(&producer, NULL, producerFunction, NULL);
    pthread_create(&consumer, NULL, consumerFunction, NULL);

    pthread_join(producer, NULL);
    sem_post(&mySyncStruct.newResult); // Wake up any remaining consumers by incrementing 1 to the internal semaphore counter for all consumers
    pthread_join(consumer, NULL);

    // Cleanup stuff
    sem_destroy(&mySyncStruct.newResult);
    sem_destroy(&mySyncStruct.newWork);

    return 0;
}

