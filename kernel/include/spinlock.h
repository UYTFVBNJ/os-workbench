#ifndef SPINLOCK
#define SPINLOCK

#include <common.h>
#include <kmt.h>

struct spinlock {
  int locked;
  
  char *name;
  cpu_t *cpu;
};

void spin_init(spinlock_t *lk, const char *name);
void spin_lock(spinlock_t *lk);
void spin_unlock(spinlock_t *lk);

#endif