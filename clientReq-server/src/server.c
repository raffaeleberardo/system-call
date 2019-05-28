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

int serverFIFO; //serverFIFO_extra

int main (int argc, char *argv[]) {
    printf("Hi, I'm Server program!\n");
    //inizio scrittura
    char pathToClientFIFO[25];
    //creazione serverFIFO
    if (mkfifo(pathToServerFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
      errExit("mkfifo failed");
    printf("<Server> serverFIFO creata con successo...\n");
    serverFIFO = open(pathToServerFIFO, O_RDONLY);
    if (serverFIFO == -1)
      errExit("open failed");

    /*serverFIFO_extra = open(pathToServerFIFO, O_RDONLY);
    if(serverFIFO_extra == -1)
      errExit("open failed");*/

    struct Request request;
    if (read(serverFIFO, &request, sizeof(request)) == -1)
      errExit("read failed");
    printf("<Server> Lettura Request eseguita con successo\n");
    sprintf(pathToClientFIFO, "%s%d", baseClientFIFO, request.processID);
    int clientFIFO = open(pathToClientFIFO, O_WRONLY);
    struct Response response;
    response.key = 123;
    if (write(clientFIFO, &response, sizeof(struct Response)) != sizeof(struct Response))
      errExit("write failed");
    printf("<Server> scrittura chiave su clientFIFO eseguita...\n");
    close(serverFIFO);
    unlink(pathToServerFIFO);
    return 0;
}
