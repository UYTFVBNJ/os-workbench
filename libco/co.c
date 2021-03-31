#include "co.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

static inline void stack_switch_call(void *sp, void *entry,
                                     uintptr_t arg) {  // TODO + ret addr
  asm volatile(
#if __x86_64__
      "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      :
      : "b"((uintptr_t)sp), "d"(entry), "a"(arg)
#else
      "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      :
      : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

co *co_current;
co *co_pool[CO_POOL_SIZE];


// invisible part

static co *co_sheduler() {
  for (int i = 0; i < CO_POOL_SIZE; i++)
    if (co_pool[i] != NULL &&
        (co_pool[i]->status == CO_NEW || co_pool[i]->status == CO_RUNNING))
      return co_pool[i];

  assert(0);
}

static void co_destroyer(co *co) {
  free(co);

  int i;
  for (i = 0; i < CO_POOL_SIZE; i++)
    if (co_pool[i] == co) {
      co_pool[i] = NULL;
      break;
    }
  assert(i < CO_POOL_SIZE);
}

static void co_base(co *co) {
  co->status = CO_RUNNING;
  co->func(co->arg);
  longjmp(co->waiter->context, 1);
}

static __attribute__((constructor)) void co_current_main() {
  co_pool[0] = co_start("main", NULL, NULL);
  co_pool[0]->status = CO_RUNNING;
  printf("co_current: %p\n", co_current);
  co_pool[0]->waiter = NULL;
  co_current = co_pool[0];
}



// visible part

// co_constructor
co *co_start(const char *name, void (*func)(void *), void *arg) {
  co *p = (co *)malloc(sizeof(co));
  assert(p != NULL);

  p->name = name;
  p->func = func;
  p->arg = arg;

  p->status = CO_NEW;
  p->waiter = co_current;

  // p->context = ;

  int i;
  for (i = 0; i < CO_POOL_SIZE; i++)
    if (co_pool[i] == NULL) {
      co_pool[i] = p;
      break;
    }
  assert(i < CO_POOL_SIZE);

  return p;
}

void co_wait(co *co) {
  assert(co != co_pool[0]);

  co_current->status = CO_WAITING;

  switch (co->status) {
    case CO_NEW:
      ;
      int ret = setjmp(co_current->context);
      if (ret == 0) stack_switch_call(co->stack + STACK_SIZE, co_base, co);
      co_destroyer(co);
      break;

    case CO_RUNNING:
    case CO_WAITING:
      co_yield();
      break;

    case CO_DEAD:
      assert(0);
      break;

    default:
      assert(0);
  }

  co_current->status = CO_RUNNING;
}

void co_yield() {  // can switch to itself
  int ret = setjmp(co_current->context);
  if (ret == 0) {
    co *co = co_sheduler();
    switch (co->status) {
      case CO_NEW:
        stack_switch_call(co->stack + STACK_SIZE, co_base, (void *)co);
        break;

      case CO_RUNNING:
        longjmp(co->context, 1);
        break;

      default:
        assert(0);
    }
  }
}

