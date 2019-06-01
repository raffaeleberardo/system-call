#define MEMORY_SIZE 10

#include <stdlib.h>
#include <stdio.h>

//INCLUDE AGGIUNTE
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#include "errExit.h"
#include "request_response_keymanager.h"
#include "shared_memory.h"

int main (int argc, char *argv[]) {
    printf("Hi, I'm ClientExec program!\n");
    if (argc < 3) {
      printf("You should give me at least 3 arguments.\nYou give me %i arguments", argc);
      return 0;
    }
    size_t sizeMemory = sizeof(struct keyManager) * MEMORY_SIZE;
    int shmid = alloc_shared_memory(81298, sizeMemory);
    struct keyManager *km = (struct keyManager *)get_shared_memory(shmid, S_IRUSR | S_IWUSR);
    printf("CLIENT EXEC\n");
    for (int i = 0; i < MEMORY_SIZE; i++) {
      printf("%i) %s\n", i+1, (km+i)->identificativo);
      printf("%i) %i\n", i+1, (km+i)->key);
      printf("%i) %lu\n", i+1, (km+i)->timestamp);
    }
    char identificativo[100];
    strcpy(identificativo, argv[1]);
    printf("Lunghezza identificativo: %li\n", strlen(identificativo));
    int key = atoi(argv[2]);
    printf("identificativo: %s\nchiave: %i", identificativo, key);
    int found = 0;
    for (int i = 0; i < MEMORY_SIZE; i++) {
      printf("strcmp: %i\n", strcmp((km+i)->identificativo, identificativo));
      if ((strcmp((km+i)->identificativo, identificativo) == 0) && ((km+i)->key == key)) {
        found = 1;
        strcpy((km+i)->identificativo, "\0");
        (km+i)->key = 0;
        (km+i)->timestamp = 0;
        break;
      }
    }
    printf("found: %i\n", found);
    if (!found){
      printf("<ClientExec> Richiesta non valida!\n");
      return 0;
    }
    //richiesta servizo STAMPA
    if (key >= 1 && (key%2)== 1) {
      char *argVec[argc-2];
      for (int i = 0; i<argc-3; i++) {
        argVec[i] = argv[i+3];
      }
      argVec[argc-3] = (char *)NULL;
      if (execv("stampa", argVec) == -1) {
        errExit("execl failed");
      }
    }
    /*
    //richiesta servizio INVIA
    else if(key >= 2 && (key%2) == 0){

    }
    //richiesta SALVA
    else{

    }*/
    return 0;
}
