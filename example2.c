#include <stdio.h>
#include <pthread.h>
#include <stdint.h>


/* Thread-Funktion */
void *example_fct(void *args){
  /* Die Übergabe wird zurück auf einen int-Pointer gecastet*/
  uintptr_t *inParam = (uintptr_t *)args;

  /* Der Inhalt des Pointers wird ausgegeben */
  printf("Infos von Main: %lu\n", *inParam);

  return (void *)101010;
}

int main(){
  /* Lege ein Thread-Handle, einen Übergabe- und einen Rückgabeparameter an */
  pthread_t thread;
  uintptr_t threadParam = 42;
  uintptr_t threadRetParam = -1;

  /* Starte einen Thread mit der auszuführenden Funktion example_fct.
  Zudem wir ein Parameter übrgeben.
  Konfigurations-Parameter werden nicht gesetzt, daher NULL. */
  pthread_create(&thread, NULL, &example_fct, &threadParam);

  /* Warte auf Beendigung des Threads */
  pthread_join(thread, (void **)(&threadRetParam));

  /* Inhalt des Rückgabeparameters ausgeben */
  printf("Rueckgabe von Thread: %lu\n", threadRetParam);

  return 0;
}
