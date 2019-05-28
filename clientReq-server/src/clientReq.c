#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//include aggiunte
#include <sys/stat.h>
#include <fcntl.h>

#include "errExit.h"
#include "request_response.h"

char *pathToServerFIFO = "/tmp/serverFIFO";
char *baseClientFIFO = "/tmp/clientFIFO.";

int main (int argc, char *argv[]) {
    printf("Hi, I'm ClientReq program!\n");

    //Inizio scrittura codice...

    //percorso clientFIFO
    char pathToClientFIFO[25];
    sprintf(pathToClientFIFO, "%s%d", baseClientFIFO, getpid());
    printf("<Client %d> Making a FIFO...\n", getpid());

    //creazione clientFIFO
    if (mkfifo(pathToClientFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
      errExit("mkdir failed");
    printf("<Client %d>FIFO creata con successo\n", getpid());

    //Stampa servizi
    printf("Servizi di sistema:\n\t1. Stampa\n\t2. Salva\n\t3 .Invia\n");

    //struct Requeste
    struct Request request;
    //input id e richiesta
    printf("Codice identificativo: ");
    scanf("%s", request.id);
    printf("Servizio: ");
    scanf("%s", request.servizio);

    //Stampa
    printf("%s\n%s\n", request.id, request.servizio);

    //apertura serverFIFO
    printf("<Client %d>Apertura serverFIFO in sola lettura...\n", getpid());
    if (open(pathToServerFIFO, O_WRONLY) == -1)
      errExit("open failed");
    printf("<Client %d>Apertura serverFIFO eseguita con successo...\n", getpid());

    //...fine scrittura
    return 0;
}
