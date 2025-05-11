#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#define MAX_STUDENT_NAME_LENGTH 100

typedef struct studentName {
  char studentName[MAX_STUDENT_NAME_LENGTH];
  uintptr_t studentNameLength;
} studentName;

/* Thread-Funktion */
  void *studentNameLengthSetterThread(void *args){
  /* Die Übergabe wird zurück auf einen int-Pointer gecastet*/
  studentName * inStudentStruct = (studentName *)args;

  inStudentStruct->studentNameLength = strlen(inStudentStruct->studentName);

  return (void *)(inStudentStruct->studentNameLength); //Thread gibt info an main zurueck
}

int main(){
  /* Lege ein Thread-Handle, einen Übergabe- und einen Rückgabeparameter an */
  pthread_t thread;
  uintptr_t threadRetParam = -1; 

  studentName myStudentName;

  strncpy(myStudentName.studentName, "Patrick", MAX_STUDENT_NAME_LENGTH);
  
  /* Starte einen Thread mit der auszuführenden Funktion studentNameLengthSetterThread. Zudem wird ein Parameter übergeben. Konfigurations-Parameter(Hier die Memory addresse des structs) werden nicht gesetzt, daher NULL. */
  pthread_create(&thread, NULL, &studentNameLengthSetterThread, &myStudentName); // Main gibt infos(threadParam) an thread. The difference here is only the last paramter threadParam that is given to the thread, besides that its only a normal thread

  /* Warte auf Beendigung des Threads. Right now we just ignore/don't use this return value but it contains the calcualted studentNameLength*/
  pthread_join(thread, (void *)(&threadRetParam)); // Return Wert des Threads wird bei Beendigung des Threads in der Variable `threadRetParam` gespeichert

  printf("Der Student heisst %s und sein Name hat %ld Zeichen(berechnet und gesetzt in einem seperatem Thread)", myStudentName.studentName, myStudentName.studentNameLength);

  return 0;
}
