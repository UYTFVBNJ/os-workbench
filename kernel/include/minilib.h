#include <common.h>
#include <spinlock.h>

#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

#define Log(num) printf("  num: %d\n", num);

typedef struct node {
  void *key;
  struct node *pre, *nxt;
} node_t;

typedef struct list {
  spinlock_t lock;
  node_t *front;
} list_t;

bool is_2_power(int64_t n);