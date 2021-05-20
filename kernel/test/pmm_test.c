#include <common.h>
#include <minilib.h>
#include <pmm_test.h>
#include <spinlock.h>

#define PRN 20

struct workload
{
  int pr[PRN], sum; // sum = pr[0] + pr[1] + ... pr[N-1]
                    // roll(0, sum-1) => allocation size
};

static struct workload wl_typical
  __attribute__((used)) = { .pr = { 0,
                                    0,
                                    0,
                                    1,
                                    1,
                                    1,
                                    1,
                                    10,
                                    0,
                                    0,
                                    40,
                                    50,
                                    40,
                                    30,
                                    20,
                                    10,
                                    4,
                                    2,
                                    1 },
                            .sum = 0 + 0 + 0 + 1 + 1 + 1 + 1 + 10 + 0 + 0 + 40 +
                                   50 + 40 + 30 + 20 + 10 + 4 + 2 + 1 },
  wl_stress
  __attribute__((used)) = { .pr = { 0,
                                    0,
                                    0,
                                    1,
                                    1,
                                    1,
                                    1,
                                    1,
                                    0,
                                    0,
                                    400,
                                    200,
                                    100,
                                    1,
                                    1,
                                    1,
                                    1,
                                    1,
                                    1 },
                            .sum = 0 + 0 + 0 + 1 + 1 + 1 + 1 + 1 + 0 + 0 + 400 +
                                   200 + 100 + 1 + 1 + 1 + 1 + 1 + 1 },
  wl_page __attribute__((used)) = {
    .pr = { 0, 0, 0, 1, 1, 1, 1, 10, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    .sum =
      0 + 0 + 0 + 1 + 1 + 1 + 1 + 10 + 0 + 0 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1
  };
static struct workload* workload = &wl_typical;

int
roll()
{
  int i, tmp = rand() % workload->sum;
  for (i = 0; i < PRN && tmp >= 0; i++) {
    tmp -= workload->pr[i];
  }
  assert(1 <= PRN - (i - 1) && PRN - (i - 1) <= PRN);
  return 1 << (PRN - (i - 1));
}

#define ALLOC_SIZE (roll())
// #define ALLOC_SIZE (1 << 14)
// #define ALLOC_SIZE (1 << 12)
// #define ALLOC_SIZE (1 << 5)
#define N ((1 << (HEAP_XFT - 12 - 2)))
// #define N 100

#define RATE 2
#define LOOP 1 << 30

// #define OUTPUT
#define CHECK

spinlock_t cnt_lk[MAX_CPU];
int cnt[MAX_CPU];

spinlock_t chk_lk;
int chk;

spinlock_t free_cnt_lk[MAX_CPU];
int free_cnt[MAX_CPU];

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
struct malloc_op op_arr[4][N];

static void
op_insert(enum ops type, size_t size, void* addr)
{
  // lock(&op_lk);
  int cpu = cpu_current();
  int i;
  for (i = 0; i < N && op_arr[cpu][i].type != OP_NONE; i++)
    ;

  assert(i < N);

  op_arr[cpu][i] =
    (struct malloc_op){ .type = type, .size = size, .addr = addr };

  // unlock(&op_lk);
}

static void
random_op(struct malloc_op* op)
{
  if (rand() % RATE) {
    // OP_ALLOC
    *op = (struct malloc_op){ .type = OP_ALLOC, .size = ALLOC_SIZE };
  } else {
    // OP_FREE
    // lock(&op_lk);
    int cpu = cpu_current();
    int i;
    for (i = 0; i < N && op_arr[cpu][i].type != OP_FREE; i++)
      ;

    if (i < N) {
      *op = op_arr[cpu][i];
      op_arr[cpu][i] = (struct malloc_op){ .type = OP_NONE };
    }

    // unlock(&op_lk);

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
#ifdef CHECK
  if (addr != NULL)
    pmm_test_paint(addr, op->size, op->size);
#endif
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
#ifdef CHECK
  pmm_test_check(op->addr, op->size, op->size);
#endif
  pmm->free(op->addr);
  free_cnt[cpu_current()]++;
#ifdef OUTPUT
  printf("cpu %d %d bytes freed at %p\n", cpu_current(), op->size, op->addr);
#endif
}

static void
stress_test()
{
  int cpu = cpu_current();
  {

    struct malloc_op op =
      (struct malloc_op){ .type = OP_ALLOC, .size = 1 << 24 };
    void* addr = alloc_check(&op);
    assert(addr != NULL);
    op_insert(OP_FREE, op.size, addr);
  }
  uint64_t time = 0;
  for (int t = 0; t < LOOP; t++) {
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
    // /*
    lock(&cnt_lk[cpu]);
    cnt[cpu]++;
    unlock(&cnt_lk[cpu]);

    if (cpu == 0 && time != uptime() / 1000000) {
      time = uptime() / 1000000;
      int tot = 0;
      for (int i = 0; i < 4; i++) {
        lock(&cnt_lk[i]);
        tot += cnt[i];
        unlock(&cnt_lk[i]);
      }
      printf("cnt: %d\ntime: %ds\nspeed: %fM op/s\n",
             tot,
             time,
             0.000001 * tot / time);
    }
    // */
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
