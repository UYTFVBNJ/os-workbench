#ifndef SPINLOCK
#define SPINLOCK

struct spinlock {
  int locked;

  // For debugging:
  char *name;
  int cpu;
};

#define LOCK_INIT(name, cpu) \
  ((spinlock){.locked = 0, .name = name, .cpu = cpu})

void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);

#endif