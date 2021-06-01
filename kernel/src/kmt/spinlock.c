#include <spinlock.h>

void spin_init(spinlock_t *lk, const char *name) {
  lk->cpu = NULL;
  lk->name = name;
}

// TODO
void spin_lock(spinlock_t *lk) {
  panic_on(holding(lk), lk->name); // "Reentrance" 

  while (atomic_xchg(&lk->locked, 1));

  lk->cpu = cpu_current();
}

void spin_unlock(spinlock_t *lk) {
  panic_on(!holding(lk), lk->name); // "cpu switched"

  atomic_xchg(&lk->locked, 0);

  lk->cpu = NULL;
}

// int off, cpu won't change
inline bool holding(spinlock_t *lk) {
  return lk->cpu == cpu_current();
}

inline bool push_noff(spinlock_t *lk) {
  if (lk->cpu->noff == 0) iset(false);
  lk->cpu->noff ++;
}

inline void pop_noff(spinlock_t *lk) {
  lk->cpu->noff --;
  if (lk->cpu->noff == 0) iset(true);
}