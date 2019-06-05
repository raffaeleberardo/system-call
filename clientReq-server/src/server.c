#define MEMORY_SIZE 10
#define WAIT -1
#define SIGNAL 1

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
#include <sys/sem.h>

#include "errExit.h"
#include "request_response_keymanager.h"
#include "shared_memory.h"
#include "semaphore.h"

char *pathToServerFIFO = "/tmp/serverFIFO";
char *baseClientFIFO = "/tmp/clientFIFO.";

int serverFIFO, serverFIFO_extra;
pid_t child = -1;

int shmid;
struct keyManager *km;

int semid;

int stampa = 1;
int salva = -2;
int invia = 2;

//FUNZIONI
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
  if (child == 0 && sig == SIGTERM) {
    printf("<keyManager> Terminazione\n");
  }
  else{
    if (sig == SIGTERM){
      kill(child, SIGTERM);
      free_shared_memory(km);
      remove_shared_memory(shmid);
      printf("<Server> Terminazione...\n");
      if(serverFIFO != 0 && close(serverFIFO) == -1){
        errExit("close failed");
      }
      if(serverFIFO_extra != 0 && close(serverFIFO_extra) == -1){
        errExit("close failed");
      }
      if (semctl(semid, 0/*ignored*/, IPC_RMID, 0/*ignored*/) == -1){
        errExit("semctl failed");
      }
      else{
        printf("semaphore set removed successfully\n");
      }
      if (unlink(pathToServerFIFO) != 0){
        errExit("unlink failed");
      }
    }
  }
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

//MAIN

int main (int argc, char *argv[]) {
  printf("Hi, I'm Server program!\n");
  //inizio scrittura
  signalManager(); //blocca tutti segnali tranne SIGTERM
  printf("<Server> Tutti segnali bloccati tranne SIGTERM (int 15)\n\n");
  //Memoria condivisa
  int contatore = 0;
  size_t sizeMemory = sizeof(struct keyManager) * MEMORY_SIZE;
  shmid = alloc_shared_memory(81298, sizeMemory);
  //attach shared memory
  km = (struct keyManager *)get_shared_memory(shmid, S_IRUSR | S_IWUSR);

  //SEMAFORO
  semid = semget(10, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  // Initialize the semaphore set
  unsigned short semInitVal[] = {1};
  union semun arg;
  arg.array = semInitVal;
  if (semctl(semid, 0 /*ignored*/, SETALL, arg) == -1){
    errExit("semctl SETALL failed");
  }

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

  //PROCESSO FIGLIO CHE CONTROLLA SHARED MEMORY OGNI 30 SECONDI
  child = fork();
  if (child != 0) {
    printf("pidof keyManager: %i\n", child);
  }
  if (child == -1) {
    printf("<Server> keyManager rotto\n");
    errExit("fork failed");
  }
  if (child == 0) {
    while (1) {
      printf("<KeyManager> Dormo...\n");
      sleep(30);
      printf("<KeyManager> Sveglio...\n");
      semOp(semid, 0, WAIT);
      printf("<KeyManager> Semaphore WAIT...\n");
      for (int i = 0; i < MEMORY_SIZE; i++) {
        if (strlen((km+i)->identificativo) != 0 && (km+i)->key != 0 && (km+i)->timestamp != 0) {
          if (difftime(time(NULL), (km+i)->timestamp) > 300) {
            strcpy((km+i)->identificativo, "\0");
            (km+i)->key = 0;
            (km+i)->timestamp = 0;
            printf("<KeyManager> Elimanta entry %i...\n", i+1);
          }
        }
      }
      printf("<KeyManager> Semaphore SIGNAL...\n");
      semOp(semid, 0, SIGNAL);
    }
  }

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
        //INSERIMENTO VALORI IN SHARED MEMORY
        printf("<Server> Contatore: %i\n", contatore);
        semOp(semid, 0, WAIT);
        printf("<Server> Semaphore WAIT...\n");
        strcpy((km+contatore)->identificativo, request.identificativo);
        (km+contatore)->key = response.key;
        (km+contatore)->timestamp = time(NULL);
        printf("<Server> Semaphore SIGNAL...\n");
        semOp(semid, 0, SIGNAL);
        contatore++;
        if (contatore >= MEMORY_SIZE)
        contatore = 0;
        //stampa shared_memory
        printf("SHARED MEMORY\n");
        for (int i = 0; i < MEMORY_SIZE; i++) {
          printf("%i) %s\n", i+1, (km+i)->identificativo);
          printf("%i) %i\n", i+1, (km+i)->key);
          printf("%i) %lu\n", i+1, (km+i)->timestamp);
        }
      }
      sendResponse(&request, &response);
    }
  } while(bR != -1);
}
