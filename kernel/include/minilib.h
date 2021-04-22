#ifndef MINILIB
#define MINILIB

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

void list_push_front(list_t *list, node_t *node);
void list_pop_front(list_t *list);

bool is_2_power(int64_t n);

int num2shift(int64_t n);

#endif