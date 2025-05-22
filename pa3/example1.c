#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

// globale Variablen
int counter = 0;
int counter2 = 0;

void* worker_func(void*) {
  for( int i = 0; i < 1000000; i++ ) {
    // unprotected increment
    counter++;
    // protected increment
    atomic_fetch_add( &counter2, 1);
  }

  return 0;
}

int main() {

pthread_t threads[8];

for( int i=0; i<8; i++ ) {
  pthread_create( &threads[i], 0, &worker_func, 0 );
};

// Warte bis alle Threads fertig sind

for( int i=0; i < 8; i++ ) {
  pthread_join( threads[i], 0 );
}

printf("Unprotected counter: %d\n", counter );
printf("Protected counter: %d\n", counter2 );

}
