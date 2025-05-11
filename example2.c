#include <stdio.h>
#include <pthread.h>
#include <stdint.h>


/* Thread-Funktion */
void *example_fct(void *args){
  /* Die Übergabe wird zurück auf einen int-Pointer gecastet*/
  uintptr_t *inParam = (uintptr_t *)args;

  /* Der Inhalt des Pointers wird ausgegeben */
  printf("Infos von Main: %lu\n", *inParam); 

  return (void *)101010; //Thread gibt info an main zurueck
}

int main(){
  /* Lege ein Thread-Handle, einen Übergabe- und einen Rückgabeparameter an */
  pthread_t thread;
  uintptr_t threadParam = 42;
  uintptr_t threadRetParam = -1;

  /* Starte einen Thread mit der auszuführenden Funktion example_fct. Zudem wir ein Parameter übergeben. Konfigurations-Parameter werden nicht gesetzt, daher NULL. */
  pthread_create(&thread, NULL, &example_fct, &threadParam); // Main gibt infos(threadParam) an thread. The difference here is only the last paramter threadParam that is given to the thread, besides that its only a normal thread

  /* Warte auf Beendigung des Threads */
  pthread_join(thread, (void **)(&threadRetParam)); // Return Wert des Threads wird bei Beendigung des Threads in der Variable `threadRetParam` gespeichert

  /* Inhalt des Rückgabeparameters ausgeben */
  printf("Rueckgabe von Thread: %lu\n", threadRetParam);

  return 0;
}
