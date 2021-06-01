#ifndef SEMAPHORE
#define SEMAPHORE

#include <common.h>
#include <kmt.h>
#include <spinlock.h>

struct semaphore {
  spinlock_t spinlock;
  int cnt;
  char *name;
};

void sem_init(sem_t *sem, const char *name, int value);
void sem_wait(sem_t *sem);
void sem_signal(sem_t *sem);

#endif