#include <stdlib.h>
#include <stdio.h>

//INCLUDE AGGIUNTE
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "errExit.h"

int main (int argc, char *argv[]) {
    printf("Hi, I'm Salva program!\n");
    if (argc <= 1) {
      printf("Nessun argomento da stampare\n");
      return 0;
    }
    printf("argc: %i\n", argc);
    printf("argv[0]: %s\n", argv[0]);
    int fd = open(argv[0], O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
    char buf[] = "\n";
    for (int i = 1; i < argc; i++) {
      if (write(fd, argv[i], strlen(argv[i])) == -1) {
        errExit("write failed");
      }
      if (write(fd, buf, strlen(buf)) == -1) {
        errExit("write failed");
      }
    }
    return 0;
}
