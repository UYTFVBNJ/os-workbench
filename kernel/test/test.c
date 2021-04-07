#include <common.h>
#include <klib.h>

enum ops { OP_ALLOC = 1, OP_FREE };
struct malloc_op {
  enum ops type;
  union { size_t sz; void *addr; };
};

void stress_test() {

  while (1) {
    struct malloc_op op = random_op();
    switch (op.type) {
      case OP_ALLOC: alloc_check(pmm->alloc(op.sz), op.sz); break;
      case OP_FREE:  free(op.addr); break;
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

/*
static void entry(int tid) { pmm->alloc(128); }
static void goodbye()      { printf("End.\n"); }
int main() {
  pmm->init();
  for (int i = 0; i < 4; i++)
    create(entry);
  join(goodbye);
}
*/