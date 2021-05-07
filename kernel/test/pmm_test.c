#include <common.h>
#include <pmm_test.h>
#include <spinlock.h>

// #define ALLOC_SIZE (1 << 12)
#define ALLOC_SIZE (1 << 5)
#define N ((1 << (HEAP_XFT - 12)) - 100)
// #define N 100

#define OUTPUT

enum ops { OP_NONE, OP_ALLOC, OP_FREE };

struct malloc_op {
  enum ops type;
  size_t size;
  void *addr;
};

spinlock_t op_lk;
struct malloc_op op_arr[N];

static void op_insert(enum ops type, size_t size, void *addr) {
  lock(&op_lk);

  int i;
  for (i = 0; i < N && op_arr[i].type != OP_NONE; i++)
    ;

  assert(i < N);

  op_arr[i] = (struct malloc_op){.type = type, .size = size, .addr = addr};

  unlock(&op_lk);
}

static void random_op(struct malloc_op *op) {
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

void pmm_test_paint(uint32_t *addr, size_t size, int key) {
  size /= sizeof(uint32_t);
  for (uint32_t *chk_ptr = addr; chk_ptr < addr + size; chk_ptr++) {
    if (*(uint32_t *)chk_ptr == USED(key)) printf("%p\n", chk_ptr);
    assert(*(uint32_t *)chk_ptr != USED(key));
    *(uint32_t *)chk_ptr = USED(key);
  }
}

void pmm_test_check(uint32_t *addr, size_t size, int key) {
  size /= sizeof(uint32_t);
  for (uint32_t *chk_ptr = addr; chk_ptr < addr + size; chk_ptr++) {
    if (*(uint32_t *)chk_ptr != USED(key)) printf("%p\n", chk_ptr);
    assert(*(uint32_t *)chk_ptr == USED(key));
    *(uint32_t *)chk_ptr = 0;
  }
}

static void *alloc_check(struct malloc_op *op) {
#ifdef OUTPUT
  printf("acquiring %d bytes\n", op->size);
#endif
  void *addr = pmm->alloc(op->size);
#ifdef OUTPUT
  printf("got %p \n", addr);
#endif
  pmm_test_paint(addr, op->size, op->size);
  return addr;
}

static void free_check(struct malloc_op *op) {
#ifdef OUTPUT
  printf("freeing mem of %d bytes at %p\n", op->size, op->addr);
#endif
  pmm->free(op->addr);
#ifdef OUTPUT
  printf("%d bytes freed at %p\n", op->size, op->addr);
  pmm_test_paint(op->addr, op->size, op->size);
#endif
}

static void stress_test() {
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
      case OP_NONE:
        assert(0);
    }
  }
}

void pmm_test() {
#ifdef TEST
  printf("PMM_TEST of cpuid %d\n", cpu_current());
#endif
  stress_test();
}
