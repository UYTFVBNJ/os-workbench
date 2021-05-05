#include <common.h>
#define USED 0x66662333
#define ALLOC_SIZE (1 << 12)
#define N 1000

enum ops { OP_NONE, OP_ALLOC, OP_FREE };
struct malloc_op {
  enum ops type;
  size_t sz;
  void *addr;
};

struct malloc_op op_arr[N];
int op_arr_cnt = 0;

struct malloc_op *random_op() {
  if (rand() % 2) {
    int i;
    for (i = 0; i < op_arr_cnt && op_arr[i].type != OP_NONE; i++)
      ;
    if (i == op_arr_cnt) op_arr_cnt++;
    assert(op_arr_cnt <= N);
    op_arr[i] = (struct malloc_op){.type = OP_ALLOC, .sz = ALLOC_SIZE};
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
  printf("acquied %d bytes\n", op->sz);
  void *addr = pmm->alloc(op->sz);
  printf("got %p \n", addr);
  for (size_t i = 0; i < op->sz; i++) {
    assert(*(uint32_t *)addr != USED);
    *(uint32_t *)addr = USED;
  }

  op->type = OP_FREE;
  op->addr = addr;
}

void free_check(struct malloc_op *op) {
  printf("free mem at %p\n", op->addr);
  pmm->free(op->addr);
  op->type = OP_NONE;
  printf("$d bytes freed\n", op->sz);
}

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
  pmm->init();

#ifdef TEST
  printf("TESTING\n");
#endif

  stress_test();
}
