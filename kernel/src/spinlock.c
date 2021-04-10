#include <common.h>
#include <spinlock.h>

void lock(spinlock_t *lk) {
//   iset(false); // TODO
  while (atomic_xchg(&lk->locked, 1)) ;
}

void unlock(spinlock_t *lk) {
  atomic_xchg(&lk->locked, 0);
//   iset(true); // TODO
}
