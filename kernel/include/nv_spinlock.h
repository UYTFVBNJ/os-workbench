#ifndef NV_SPINLOCK
#define NVSPINLOCK

typedef struct {
  int locked;
} nv_spinlock_t;

#define LOCK_INIT(name, cpu) \
  ((spinlock){.locked = 0, .name = name, .cpu = cpu})

void nv_lock(nv_spinlock_t *lk);
void nv_unlock(nv_spinlock_t *lk);

#endif