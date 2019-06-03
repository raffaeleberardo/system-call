#ifndef SEMUN_H
#define SEMUN_H
#include <sys/sem.h>
// definition of the union semun
union semun {
  int val;
  struct semid_ds * buf;
  unsigned short * array;
};

void semOp(int semid, unsigned short sem_num, short sem_op);

#endif
