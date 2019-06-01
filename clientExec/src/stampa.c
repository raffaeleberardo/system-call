#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
    printf("Hi, I'm Stampa program!\n");
    printf("argc: %i\n", argc);
    if (argc <= 1) {
      printf("Nessun argomento da stampare\n");
      return 0;
    }
    for (int i = 0; i<argc; i++) {
      printf("%s ", *(argv+i));
    }
    printf("\n");
    return 0;
}
