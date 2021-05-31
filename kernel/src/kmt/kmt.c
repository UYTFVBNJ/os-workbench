#include <kmt.h>

void kmt_init() {
  assert(0);
}

// Parallel
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
  TRACE_ENTRY;
  task->stack = pmm->alloc(STACK_SIZE); 

  TRACE_EXIT_with(0);
}

// Parallel
void kmt_teardown(task_t *task) {
  TRACE_ENTRY;
  pmm->free(task->stack); 

  TRACE_EXIT;
}

MODULE_DEF(kmt) = {
  .init = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
};