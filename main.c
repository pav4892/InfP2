#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>


/* Thread-Funktion */
void *example_fct(void *args){
  /* Die Übergabe wird zurück auf einen int-Pointer gecastet*/
  char * inParam = (char *)args;

  uintptr_t inParamLength = strlen(inParam);

  /* Der Inhalt des Pointers wird ausgegeben */
  printf("Infos von Main: %s has %ld chars\n", inParam, inParamLength); 

  return (void *)inParamLength; //Thread gibt info an main zurueck
}

int main(){
  /* Lege ein Thread-Handle, einen Übergabe- und einen Rückgabeparameter an */
  pthread_t thread;
  uintptr_t threadRetParam = -1;
  uint16_t MAX_STUDENT_NAME_LENGTH = 100;

  typedef struct studentName {
    char studentName[MAX_STUDENT_NAME_LENGTH];
    uint16_t studentNameLength;
  } studentName;

  studentName myStudentName;

  strncpy(myStudentName.studentName, "Patrick", MAX_STUDENT_NAME_LENGTH);
  
  /* Starte einen Thread mit der auszuführenden Funktion example_fct. Zudem wir ein Parameter übergeben. Konfigurations-Parameter werden nicht gesetzt, daher NULL. */
  pthread_create(&thread, NULL, &example_fct, myStudentName.studentName); // Main gibt infos(threadParam) an thread. The difference here is only the last paramter threadParam that is given to the thread, besides that its only a normal thread

  /* Warte auf Beendigung des Threads */
  pthread_join(thread, (void *)(&threadRetParam)); // Return Wert des Threads wird bei Beendigung des Threads in der Variable `threadRetParam` gespeichert
  myStudentName.studentNameLength = threadRetParam;

  /* Inhalt des Rückgabeparameters ausgeben */
  printf("Rueckgabe von Thread: %lu\n", threadRetParam);

  return 0;
}
