#include <kmt.h>

void kmt_init() {
  assert(0);
}

int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
  TRACE_ENTRY;
  task->stack = pmm->alloc(STACK_SIZE); // 动态分配内核栈

  TRACE_EXIT_with(0);
}

void kmt_teardown(task_t *task) {}

MODULE_DEF(kmt) = {
  .init = kmt_init,
  .create = kmt_create,
  .teardown = kmt_teardown,
};