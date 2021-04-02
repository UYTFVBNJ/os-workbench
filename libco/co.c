#include <setjmp.h>
#include <stdint.h>

#include "co.h"

enum co_status{
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

#define STACK_SIZE 4096 * 1024
#define CO_POOL_SIZE 128

struct co {
  const char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

typedef struct co co;

#include <assert.h>
#include <stdlib.h>

#include <stdio.h>

static inline void stack_switch_call(void *sp, void *entry,
                                     uintptr_t arg) {
  assert(((uintptr_t)sp & 8) == 8);
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
  int base = rand() % CO_POOL_SIZE;
  for (int i = 0; i < CO_POOL_SIZE; i++) {
    int j = (base + i) % CO_POOL_SIZE;
    if (co_pool[j] != NULL &&
        (co_pool[j]->status == CO_NEW || co_pool[j]->status == CO_RUNNING))
      return co_pool[j];
  }

  assert(0);
}

static void co_destroyer(co *co) {
  free((char *)co - 8);

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
  co->status = CO_DEAD;
  if (co->waiter != NULL) co->waiter->status = CO_RUNNING;
  // printf("co_base\n");
  co_yield();
  // longjmp(co->waiter->context, 1);
}

__attribute__((constructor)) void co_current_main() {
  co_pool[0] = co_start("main", NULL, NULL);
  co_pool[0]->status = CO_RUNNING;
  // printf("co_current: %p\n", co_current);
  co_pool[0]->waiter = NULL;
  co_current = co_pool[0];
}

// visible part

// co_constructor
co *co_start(const char *name, void (*func)(void *), void *arg) {
  co *p = (co *)((char *)aligned_alloc(16, sizeof(co) + 16 * sizeof(char)) + 8);
  assert(p != NULL);
  // printf("%p\n", p);

  p->name = name;
  p->func = func;
  p->arg = arg;

  p->status = CO_NEW;
  p->waiter = NULL;

  // p->context = ;

  int i;
  for (i = 0; i < CO_POOL_SIZE; i++)
    if (co_pool[i] == NULL) {
      co_pool[i] = p;
      printf("%d\n", i);
      break;
    }
  assert(i < CO_POOL_SIZE);

  return p;
}

void co_wait(co *co) {
  // printf("co_wait\n");
  assert(co != co_pool[0]);

  co_current->status = CO_WAITING;

  switch (co->status) {
    case CO_NEW:;
      int ret = setjmp(co_current->context);
      // printf("%p %p %p\n", co, co->stack, co->stack + STACK_SIZE);
      if (ret == 0) {
        co->waiter = co_current;
        co_current = co;
        stack_switch_call(co->stack + STACK_SIZE, co_base, (uintptr_t)co);
      }
      co_destroyer(co);
      break;

    case CO_RUNNING:
    case CO_WAITING:
      co->waiter = co_current;
      co_yield();
      break;

    case CO_DEAD:
      co_destroyer(co);
      break;

    default:
      assert(0);
  }
}

void co_yield() {  // can switch to itself
  // printf("co_yielding\n");

  int ret = setjmp(co_current->context);
  if (ret == 0) {
    co *co = co_sheduler();
    switch (co->status) {
      case CO_NEW:
        co_current = co;
        stack_switch_call(co->stack + STACK_SIZE, co_base, (uintptr_t)co);
        break;

      case CO_RUNNING:
        co_current = co;
        longjmp(co->context, 1);
        break;

      default:
        assert(0);
    }
  }
}
