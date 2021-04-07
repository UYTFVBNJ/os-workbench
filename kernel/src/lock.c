#include <common.h>

typedef struct {
  int locked;
} spinlock_t;

#define LOCK_INIT()     ((spinlock_t) { .locked = 0 })

void spin_lock(spinlock_t *lk) {
//   iset(false); // TODO
  while (atomic_xchg(&lk->locked, 1)) ;
}

void spin_unlock(spinlock_t *lk) {
  atomic_xchg(&lk->locked, 0);
//   iset(true); // TODO
}
