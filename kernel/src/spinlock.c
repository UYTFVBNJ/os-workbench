#include <common.h>
#include <spinlock.h>

void lock(spinlock_t *lk) {
//   iset(false); // TODO
  int cnt = 0;
  while (atomic_xchg(&lk->locked, 1)) { assert(cnt ++ < 100000000); };
}

void unlock(spinlock_t *lk) {
  atomic_xchg(&lk->locked, 0);
//   iset(true); // TODO
}
