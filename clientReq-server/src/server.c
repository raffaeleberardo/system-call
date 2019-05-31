#include <stdlib.h>
#include <stdio.h>

//include aggiunte
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/shm.h>
#include <time.h>
#include <string.h>

#include "errExit.h"
#include "request_response_keymanager.h"
#include "shared_memory.h"

char *pathToServerFIFO = "/tmp/serverFIFO";
char *baseClientFIFO = "/tmp/clientFIFO.";

int serverFIFO, serverFIFO_extra;
int stampa = 1;
int salva = -2;
int invia = 0;

void signalManager(void){
  sigset_t set;
  if(sigfillset(&set) == -1)
    errExit("sigfillset failed");
  if(sigdelset(&set, SIGTERM) == -1)
    errExit("sidelset failed");
  if (sigprocmask(SIG_SETMASK, &set, NULL) == -1)
    errExit("sigprocmask failed");
}

void quit(int sig){
  if (sig == SIGTERM)
    printf("<Server> Terminazione...\n");
  if(serverFIFO != 0 && close(serverFIFO) == -1)
    errExit("close failed");
  if(serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
    errExit("close failed");
  if (unlink(pathToServerFIFO) != 0)
    errExit("unlink failed");
  _exit(0);
}

void keyCreation(struct Request *request, struct Response *response){
  if (strcmp(request->servizio, "stampa") == 0) {
    stampa += 2;
    response->key = stampa;
  }
  else if (strcmp(request->servizio, "salva") == 0) {
    salva -= 2;
    response->key = salva;
  }
  else if (strcmp(request->servizio, "invia") == 0) {
    invia += 2;
    response->key = invia;
  }
  else{
    response->key = -1;
  }
}

void sendResponse(struct Request *request, struct Response *response){
  char pathToClientFIFO[25];
  sprintf(pathToClientFIFO, "%s%d", baseClientFIFO, request->processID);
  int clientFIFO = open(pathToClientFIFO, O_WRONLY);
  if (clientFIFO == -1){
    errExit("open failed");
    return;
  }

  if (write(clientFIFO, response, sizeof(struct Response)) != sizeof(struct Response))
    errExit("write failed");
  printf("<Server> scrittura chiave su clientFIFO eseguita...\n");
  if (close(clientFIFO) != 0)
    printf("close failed\n");
}

int main (int argc, char *argv[]) {
    printf("Hi, I'm Server program!\n");
    //inizio scrittura
    signalManager(); //blocca tutti segnali tranne SIGTERM
    printf("<Server> Tutti segnali bloccati tranne SIGTERM (int 15)\n\n");
    //Memoria condivisa che salva fino a 10 richieste
    int contatore = 0;
    int n = 10;
    size_t sizeMemory = sizeof(struct keyManager) * n;
    int shmid = alloc_shared_memory(IPC_PRIVATE, sizeMemory);
    //attach shared memory
    struct keyManager *km = (struct keyManager *)get_shared_memory(shmid, S_IRUSR | S_IWUSR);
    //creazione serverFIFO
    if (mkfifo(pathToServerFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
      errExit("mkfifo failed");
    printf("<Server> serverFIFO creata con successo...\n");
    if (signal(SIGTERM, quit) == SIG_ERR)
      errExit("signal failed");
    serverFIFO = open(pathToServerFIFO, O_RDONLY);
    //apertura FIFO server in sola scrittura così che non veda la fine del file...
    if (serverFIFO == -1)
      errExit("open failed");
    serverFIFO_extra = open(pathToServerFIFO, O_WRONLY);
    if (serverFIFO_extra == -1)
      errExit("open failed");
    struct Request request;
    ssize_t bR = -1;
    do {
      printf("<Server> Attesa richieste...\n");
      bR = read(serverFIFO, &request, sizeof(request));
      if (bR == -1)
        printf("<Server>FIFO non funzionante...\n");
      else if(bR != sizeof(struct Request) || bR == 0)
        printf("<Server> Non ho ricevuto una richiesta valida...\n");
      else{
        printf("<Server> Lettura Request eseguita con successo...\n");
        struct Response response;
        keyCreation(&request, &response);
        if(response.key == -1)
          printf("<Server> Servizio non disponibile...\n");
        else{
          strcpy((km+contatore)->identificativo, request.identificativo);
          (km+contatore)->key = response.key;
          (km+contatore)->timestamp = time(NULL);
          contatore++;
          if (contatore >= n)
            contatore = 0;
          //stampa shared_memory
          printf("SHARED MEMORY\n");
          for (int i = 0; i < n; i++) {
            printf("%i) %s\n", i, (km+i)->identificativo);
            printf("%i) %i\n", i, (km+i)->key);
            printf("%i) %lu\n", i, (km+i)->timestamp);
          }
        }
        sendResponse(&request, &response);
      }
    } while(bR != -1);
}
