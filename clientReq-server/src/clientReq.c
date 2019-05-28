#include <stdlib.h>
#include <stdio.h>

//include aggiunte
#include <unistd.h>
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
    printf("<Client %d>clientFIFO creata con successo\n", getpid());
    //apertura serverFIFO
    int serverFIFO = open(pathToServerFIFO, O_WRONLY);
    if (serverFIFO == -1)
      errExit("open failed");
    printf("<Client %d>apertura serverFIFO eseguita con successo...\n", getpid());
    //Stampa servizi
    printf("Servizi di sistema:\n\t1. Stampa\n\t2. Salva\n\t3 .Invia\n");

    //struct Request
    struct Request request;
    request.processID = getpid();
    printf("Codice identificativo: ");
    scanf("%s", request.id);
    printf("Servizio: ");
    scanf("%s", request.servizio);

    //invio richiesta su serverFIFO
    if (write(serverFIFO, &request, sizeof(struct Request)) != sizeof(struct Request))
      errExit("write failed");
    printf("<Client %d>scrittura su serverFIFO eseguita con successo...\n", getpid());

    int clientFIFO = open(pathToClientFIFO, O_RDONLY);
    if (clientFIFO == -1)
      errExit("open failed");
    printf("<Client %d>Apertura clientFIFO eseguita con successo...\n", getpid());
    struct Response response;
    if(read(clientFIFO, &response, sizeof(struct Response)) != sizeof(struct Response))
      errExit("read failed");
    printf("<Client %d>Lettura eseguita con successo...\n", getpid());
    printf("Chiave rilasciata dal server: %i\n", response.key);
    close(clientFIFO);
    unlink(pathToClientFIFO);
    //...fine scrittura
    return 0;
}
