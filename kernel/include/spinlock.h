#ifndef SPINLOCK
#define SPINLOCK

typedef struct {
  int locked;

  // For debugging:
  char *name;
  int cpu;
} spinlock_t;

#define LOCK_INIT(name, cpu) \
  ((spinlock_t){.locked = 0, .name = name, .cpu = cpu})

void lock(spinlock_t *lk);
void unlock(spinlock_t *lk);

#endif