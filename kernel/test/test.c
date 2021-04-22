#include <common.h>
#include <klib.h>
#include <test.h>

#define N 1000
#define HEAP_XFT 14
#define HEAP_SIZE (1 << HEAP_XFT)

enum ops { OP_NONE, OP_ALLOC, OP_FREE };
struct malloc_op {
  enum ops type;
  union {
    size_t sz;
    void *addr;
  };
};

struct malloc_op op_arr[N];
int op_arr_cnt = 0;

struct malloc_op *random_op() {
  if (rand() % 2) {
    int i;
    for (i = 0; i < op_arr_cnt && op_arr[i].type != OP_NONE; i++)
      ;
    if (i == op_arr_cnt) op_arr_cnt++;
    op_arr[i] = (struct malloc_op){.type = OP_ALLOC, .sz = 32};
    return &op_arr[i];
  } else {
    int i;
    for (i = 0; i < op_arr_cnt && op_arr[i].type != OP_FREE; i++)
      ;
    if (i == op_arr_cnt)
      return random_op();
    else
      return &op_arr[i];
  }
}

void alloc_check(struct malloc_op *op) {
  void *addr = pmm->alloc(op->sz);
  for (size_t i = 0; i < op->sz; i++) {
    asser(*(char *)addr != USED);
    *(char *)addr = USED;
  }

  op->type = OP_FREE;
  op->addr = addr;
}

void free_check(struct malloc_op *op) { pmm->free(op->addr); }

void stress_test() {
  while (1) {
    struct malloc_op *op = random_op();
    switch (op->type) {
      case OP_ALLOC:
        alloc_check(op);
        break;
      case OP_FREE:
        free_check(op);
        break;
    }
  }
}

int main() {
  os->init();

#ifdef TEST
  printf("TESTING\n");
#endif

  mpe_init(stress_test);
}
