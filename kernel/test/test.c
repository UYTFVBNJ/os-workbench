#include <common.h>
#include <spinlock.h>
#include <threads.h>

#define ALLOC_SIZE (1 << 12)
// #define N ((1 << (HEAP_XFT - 12)) - 100)
// #define N 100
#define SMP 4

enum ops { OP_NONE, OP_ALLOC, OP_FREE };

struct malloc_op {
  enum ops type;
  size_t size;
  void *addr;
};

spinlock_t op_lk;
struct malloc_op op_arr[N];

void op_insert(enum ops type, size_t size, void *addr) {
  lock(&op_lk);

  int i;
  for (i = 0; i < N && op_arr[i].type != OP_NONE; i++)
    ;

  assert(i < N);

  op_arr[i] = (struct malloc_op){.type = type, .size = size, .addr = addr};

  unlock(&op_lk);
}

void random_op(struct malloc_op *op) {
  if (rand() % 3) {
    // OP_ALLOC
    *op = (struct malloc_op){.type = OP_ALLOC, .size = ALLOC_SIZE};
  } else {
    // OP_FREE
    lock(&op_lk);

    int i;
    for (i = 0; i < N && op_arr[i].type != OP_FREE; i++)
      ;

    if (i < N) {
      *op = op_arr[i];
      op_arr[i] = (struct malloc_op){.type = OP_NONE};
    }

    unlock(&op_lk);

    if (i == N) random_op(op);
  }
}

void *alloc_check(struct malloc_op *op) {
  printf("acquiring %d bytes\n", op->size);
  void *addr = pmm->alloc(op->size);
  printf("got %p \n", addr);
  return addr;
}

void free_check(struct malloc_op *op) {
  printf("freeing mem of %d bytes at %p\n", op->size, op->addr);
  pmm->free(op->addr);
  printf("%d bytes freed at %p\n", op->size, op->addr);
}

void stress_test() {
  printf("STRESS_TEST\n");

  while (1) {
    struct malloc_op op;
    random_op(&op);

    switch (op.type) {
      case OP_ALLOC:;
        void *addr = alloc_check(&op);
        if (addr != NULL) op_insert(OP_FREE, op.size, addr);
        break;
      case OP_FREE:
        free_check(&op);
        break;
    }
  }
}

int main() {
  pmm->init();

#ifdef TEST
  printf("TESTING:\nsmp = %d\n", SMP);
#endif

  for (int i = 0; i < SMP; i++) create(stress_test);

  join(NULL);

  // stress_test();
}
