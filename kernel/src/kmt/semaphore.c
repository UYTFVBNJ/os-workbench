#include <semaphore.h>

void sem_init(sem_t *sem, const char *name, int value) {
  spin_init(&sem->spinlock, name);
  sem->name = name;
  sem->cnt = value;
}

void sem_wait(sem_t *sem) {
  spin_lock(&sem->spinlock);
  
}

void sem_signal(sem_t *sem) {

}

