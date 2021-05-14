#include <common.h>
#include <pmm_test.h>
#include <spinlock.h>

// #define ALLOC_SIZE (1 << 14)
#define ALLOC_SIZE (1 << 12)
// #define ALLOC_SIZE (1 << 5)
#define N ((1 << (HEAP_XFT - 12)) - 100)
// #define N 100

// struct workload
// {
//   int pr[16], sum; // sum = pr[0] + pr[1] + ... pr[N-1]
//                    // roll(0, sum-1) => allocation size
// };

// static struct workload
//   wl_typical = { .pr = { 10, 0, 0, 40, 50, 40, 30, 20, 10, 4, 2, 1 } },
//   wl_stress = { .pr = { 1, 0, 0, 400, 200, 100, 1, 1, 1, 1, 1, 1 } },
//   wl_page = { .pr = { 10, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 } };
// static struct workload* workload = &wl_typical;

#define RATE 2

// #define OUTPUT

spinlock_t cnt_lk;
int cnt;

spinlock_t chk_lk;
int chk;

spinlock_t free_cnt_lk;
int free_cnt;

enum ops
{
  OP_NONE,
  OP_ALLOC,
  OP_FREE
};

struct malloc_op
{
  enum ops type;
  size_t size;
  void* addr;
};

spinlock_t op_lk;
struct malloc_op op_arr[N];

static void
op_insert(enum ops type, size_t size, void* addr)
{
  lock(&op_lk);

  int i;
  for (i = 0; i < N && op_arr[i].type != OP_NONE; i++)
    ;

  assert(i < N);

  op_arr[i] = (struct malloc_op){ .type = type, .size = size, .addr = addr };

  unlock(&op_lk);
}

static void
random_op(struct malloc_op* op)
{
  if (rand() % RATE) {
    // OP_ALLOC
    *op = (struct malloc_op){ .type = OP_ALLOC, .size = ALLOC_SIZE };
  } else {
    // OP_FREE
    lock(&op_lk);

    int i;
    for (i = 0; i < N && op_arr[i].type != OP_FREE; i++)
      ;

    if (i < N) {
      *op = op_arr[i];
      op_arr[i] = (struct malloc_op){ .type = OP_NONE };
    }

    unlock(&op_lk);

    if (i == N)
      random_op(op);
  }
}

void
pmm_test_paint(int32_t* addr, size_t size, int key)
{
  size /= sizeof(int32_t);
  lock(&chk_lk);
  chk++;
  assert(chk == 1);
  for (int32_t* chk_ptr = addr; chk_ptr < addr + size; chk_ptr++) {
    if (*chk_ptr == USED(key))
      printf("double alloc at %p : %d\n", chk_ptr, *chk_ptr);
    assert(*chk_ptr != USED(key));
    *chk_ptr = USED(key);
  }
  chk--;
  unlock(&chk_lk);
}

void
pmm_test_check(int32_t* addr, size_t size, int key)
{
  size /= sizeof(int32_t);
  lock(&chk_lk);
  chk++;
  assert(chk == 1);
  for (int32_t* chk_ptr = addr; chk_ptr < addr + size; chk_ptr++) {
    if (*chk_ptr != USED(key))
      printf("double free at %p : %d\n", chk_ptr, *chk_ptr);
    assert(*chk_ptr == USED(key));
    *chk_ptr = 0;
  }
  chk--;
  unlock(&chk_lk);
}

static void*
alloc_check(struct malloc_op* op)
{
#ifdef OUTPUT
  printf("cpu %d acquiring %d bytes\n", cpu_current(), op->size);
#endif
  void* addr = pmm->alloc(op->size);
  if (addr != NULL)
    pmm_test_paint(addr, op->size, op->size);
#ifdef OUTPUT
  printf("cpu %d got %p \n", cpu_current(), addr);
#endif
  return addr;
}

static void
free_check(struct malloc_op* op)
{
#ifdef OUTPUT
  printf("cpu %d freeing mem of %d bytes at %p\n",
         cpu_current(),
         op->size,
         op->addr);
#endif
  pmm_test_check(op->addr, op->size, op->size);
  pmm->free(op->addr);
  lock(&free_cnt_lk);
  free_cnt++;
  unlock(&free_cnt_lk);
#ifdef OUTPUT
  printf("cpu %d %d bytes freed at %p\n", cpu_current(), op->size, op->addr);
#endif
}

static void
stress_test()
{
  while (1) {
    struct malloc_op op;
    random_op(&op);

    switch (op.type) {
      case OP_ALLOC:;
        void* addr = alloc_check(&op);
        if (addr != NULL)
          op_insert(OP_FREE, op.size, addr);
        break;
      case OP_FREE:
        free_check(&op);
        break;
      case OP_NONE:
        assert(0);
    }

    lock(&cnt_lk);
    cnt++;
    if (cnt % 100000 == 0) {
      lock(&free_cnt_lk);
      printf("cnt: %d\nfree: %d\n\n", cnt, free_cnt);
      unlock(&free_cnt_lk);
    }
    unlock(&cnt_lk);
  }
}

void
pmm_test()
{
#ifdef TEST
  printf("PMM_TEST of cpuid %d\n", cpu_current());
#endif
  stress_test();
}
