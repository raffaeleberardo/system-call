#include <stdlib.h>
#include <stdio.h>

//INCLUDE AGGIUNTE
#include <sys/msg.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "errExit.h"

struct mymsg{
  long mtype;
  char buf[100];
};

int main (int argc, char *argv[]) {
  printf("Hi, I'm Invia program!\n");
  if (argc <= 1) {
    printf("Numero argomenti insufficienti\n");
    return 0;
  }
  //invio
  struct mymsg m;
  int msqid = msgget(atoi(argv[0]), IPC_CREAT | S_IRUSR | S_IWUSR);
  if (msqid == -1) {
    errExit("msgget failed");
  }
  m.mtype = 1;
  m.buf[0] = '\0';
  for (int i = 1; i < argc; i++) {
    strcat(m.buf, argv[i]);
    strcat(m.buf, " ");
  }
  strcat(m.buf, "\n");
  size_t mSize = sizeof(struct mymsg) - sizeof(long);
  if (msgsnd(msqid, &m, mSize, 0) == -1) {
    errExit("msgsnd failed");
  }
  printf("Parametri in coda messaggi\n");
  return 0;
}
