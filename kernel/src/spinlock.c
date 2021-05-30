#include <common.h>
#include <spinlock.h>

void nv_lock(nv_spinlock_t* lk) {
#ifdef L1_TEST
  int cnt = 0;
#endif

  while (atomic_xchg(&lk->locked, 1)) {

#ifdef L1_TEST
    assert(cnt++ < 100000000);
#endif
  }
}

void nv_unlock(nv_spinlock_t* lk) {
  atomic_xchg(&lk->locked, 0);
}
